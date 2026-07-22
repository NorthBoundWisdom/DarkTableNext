#include "ravo/adapters/legacy_xmp.h"

#include <limits>
#include <string>

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QXmlStreamReader>

namespace ravo
{

namespace
{

[[nodiscard]] std::string utf8(const QStringView value)
{
    return value.toString().toUtf8().toStdString();
}

[[nodiscard]] Result<void> validate_asset(const AssetDescriptor &asset)
{
    if (asset.id.empty() || asset.input_uri.empty())
    {
        return make_error(ErrorCode::kValidation,
                          "Legacy XMP import requires an explicit asset ID and input URI");
    }
    return {};
}

} // namespace

Result<Recipe> import_legacy_xmp(const LegacyXmpImportRequest &request)
{
    auto valid_asset = validate_asset(request.asset);
    if (!valid_asset)
    {
        return valid_asset.error();
    }
    if (request.xmp_utf8.size() > static_cast<std::size_t>(std::numeric_limits<qsizetype>::max()))
    {
        return make_error(ErrorCode::kValidation, "Legacy XMP document is too large");
    }

    const QByteArray source(request.xmp_utf8.data(),
                            static_cast<qsizetype>(request.xmp_utf8.size()));
    QXmlStreamReader reader(source);
    bool found_description = false;
    bool in_history = false;
    while (!reader.atEnd())
    {
        reader.readNext();
        if (reader.isStartElement())
        {
            if (reader.name() == u"Description")
            {
                found_description = true;
            }
            if (reader.name() == u"history")
            {
                in_history = true;
                continue;
            }
            if (!in_history || reader.name() != u"li")
            {
                continue;
            }
            for (const auto &attribute : reader.attributes())
            {
                if (attribute.name() == u"operation")
                {
                    return make_error(ErrorCode::kUnsupported,
                                      "Legacy XMP operation has no proven canonical recipe mapping",
                                      {{"legacy_operation", utf8(attribute.value())},
                                       {"reason", "unsupported_legacy_operation"}});
                }
            }
        }
        else if (reader.isEndElement() && reader.name() == u"history")
        {
            in_history = false;
        }
    }
    if (reader.hasError())
    {
        return make_error(ErrorCode::kValidation, "Legacy XMP is not well-formed XML",
                          {{"column", std::to_string(reader.columnNumber())},
                           {"line", std::to_string(reader.lineNumber())},
                           {"xml_error", reader.errorString().toUtf8().toStdString()}});
    }
    if (!found_description)
    {
        return make_error(ErrorCode::kValidation, "Legacy XMP does not contain an RDF description");
    }
    return Recipe{1, request.asset, {}, {}};
}

} // namespace ravo

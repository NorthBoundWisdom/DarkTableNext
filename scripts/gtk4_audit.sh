#!/usr/bin/env bash
#
# Static GTK 3 -> GTK 4 migration inventory for DarkTableNext.
#
# This script intentionally scans only first-party source under src/. It never
# reads FreeCM or generated build/dependency_* trees, so its output is suitable
# for tracking migration progress in TODO_GTK4_MIGRATION.md.

set -euo pipefail

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)
repo_root=$(cd "${script_dir}/.." && pwd -P)
source_dir="${repo_root}/src"
format="markdown"

usage()
{
  cat <<'EOF'
Usage: scripts/gtk4_audit.sh [--format markdown|plain]

Print a source-only inventory of GTK 3 APIs that require attention during the
GTK 4 migration. The script is read-only and excludes FreeCM and build trees.
EOF
}

while (($#)); do
  case "$1" in
    --format)
      if (($# < 2)); then
        echo "--format requires an argument" >&2
        exit 2
      fi
      format=$2
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

case "${format}" in
  markdown|plain) ;;
  *)
    echo "unsupported format: ${format}" >&2
    exit 2
    ;;
esac

if ! command -v rg >/dev/null 2>&1; then
  echo "gtk4 audit requires ripgrep (rg)" >&2
  exit 127
fi

if [[ ! -d "${source_dir}" ]]; then
  echo "source directory not found: ${source_dir}" >&2
  exit 1
fi

source_files=()
while IFS= read -r -d '' relative_path; do
  case "${relative_path}" in
    *.c|*.h|*.cc|*.cpp|*.cxx|*.hpp|*.m|*.mm)
      source_files+=("${repo_root}/${relative_path}")
      ;;
  esac
done < <(git -C "${repo_root}" ls-files -z -- src)

if ((${#source_files[@]} == 0)); then
  echo "no tracked C/C++/Objective-C(++) sources found under src/" >&2
  exit 1
fi

count_matches()
{
  local pattern=$1
  local output

  output=$(rg --count-matches -- "${pattern}" "${source_files[@]}" || true)
  if [[ -z "${output}" ]]; then
    printf '0\n'
    return
  fi

  printf '%s\n' "${output}" | awk -F: '{ total += $NF } END { print total + 0 }'
}

count_files()
{
  local pattern=$1
  local output

  output=$(rg -l -- "${pattern}" "${source_files[@]}" || true)
  if [[ -z "${output}" ]]; then
    printf '0\n'
    return
  fi

  printf '%s\n' "${output}" | awk 'END { print NR }'
}

labels=(
  '直接 GTK/GDK 头包含'
  '自定义 GTK 类型定义'
  'TreeView/TreeModel/cell renderer'
  '旧 GdkEvent 类型或回调'
  '显式 widget 事件 mask'
  'GtkMenu/MenuShell'
  '同步 gtk_dialog_run'
  'GtkFileChooser'
  'GtkContainer 通用 API'
  'GtkBox pack API'
  'show_all/widget destroy'
  'GdkWindow/widget_get_window'
  '旧拖放/selection data'
  'allocation/旧尺寸 API'
  'GtkStyleContext 旧 getter'
  '旧 GTK main loop API'
)

patterns=(
  '^#include <(gtk|gdk)/'
  'G_DEFINE_TYPE\([^,]+,[^,]+, GTK_TYPE_'
  'GtkTree(View|Model|Store|Path|Iter|Selection|ViewColumn)|GtkCellRenderer|gtk_tree_'
  'GdkEvent(Button|Motion|Scroll|Key|Crossing|Configure|WindowState)|GdkEvent \*'
  'gtk_widget_(add_events|set_events)'
  'GtkMenu|gtk_menu_|gtk_menu_shell_'
  'gtk_dialog_run'
  'GtkFileChooser|gtk_file_chooser_'
  'gtk_container_(add|remove|get_children|foreach)'
  'gtk_box_pack_(start|end)'
  'gtk_widget_(show_all|destroy)'
  'gtk_widget_get_window|GdkWindow|gdk_window_'
  'GdkDragContext|gtk_drag_|gtk_selection_|GtkSelectionData'
  'gtk_widget_get_allocation|GtkAllocation|gtk_widget_class_set_(get_preferred|size_allocate)'
  'GtkStyleContext|gtk_style_context_'
  'gtk_main\(|gtk_main_quit|gtk_events_pending|gtk_main_iteration'
)

if [[ "${format}" == 'markdown' ]]; then
  printf '# GTK 4 静态迁移审计\n\n'
  printf '范围：`src/` 自有 C/C++/Objective-C(++) 源码；不扫描 FreeCM、构建树或依赖 checkout。\n\n'
  printf '| 类别 | 匹配数 | 文件数 |\n'
  printf '| --- | ---: | ---: |\n'
fi

for index in "${!labels[@]}"; do
  matches=$(count_matches "${patterns[index]}")
  files=$(count_files "${patterns[index]}")

  if [[ "${format}" == 'markdown' ]]; then
    printf '| %s | %s | %s |\n' "${labels[index]}" "${matches}" "${files}"
  else
    printf '%s\t%s\t%s\n' "${labels[index]}" "${matches}" "${files}"
  fi
done

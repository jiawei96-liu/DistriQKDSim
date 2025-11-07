#!/usr/bin/env bash
set -euo pipefail

VER="0.49.0"
CDN="https://cdn.jsdelivr.net/npm/monaco-editor@${VER}/min"
OUT_DIR="./public/monaco/vs"

files=(
  "vs/loader.js"
  "vs/editor/editor.main.js"
  "vs/editor/editor.main.css"
  "vs/editor/editor.main.nls.js"
  "vs/basic-languages/cpp/cpp.js"
  "vs/base/worker/workerMain.js"
  "vs/base/common/worker/simpleWorker.nls.js"
  "vs/base/browser/ui/codicons/codicon/codicon.ttf"
  "vs/basic-languages/cpp/cpp.js"
)

echo "Output -> ${OUT_DIR}"
mkdir -p "${OUT_DIR}"

for f in "${files[@]}"; do
  url="${CDN}/${f}"
  dst="${OUT_DIR}/${f#vs/}"      # 去掉前缀 vs/，保持子目录结构
  mkdir -p "$(dirname "${dst}")"
  echo "Downloading ${url}"
  curl -fsSL "${url}" -o "${dst}"
done

echo "Done."

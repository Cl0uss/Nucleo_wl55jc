#!/usr/bin/env bash
set -e

cd ~/zephyrproject/Nucleo_wl55jc

# Находим все подпапки, кроме build и .vscode
projects=( $(find . -maxdepth 1 -type d ! -name ".*" ! -name "build" -printf "%f\n" | sort) )

# Если ничего не нашли
if [ ${#projects[@]} -eq 0 ]; then
  echo "❌ Нет доступных проектов в $(pwd)"
  exit 1
fi

# Выводим список
echo "📦 Найденные проекты:"
for i in "${!projects[@]}"; do
  echo "$((i+1))) ${projects[$i]}"
done

# Запрашиваем выбор
echo -n "Введите номер проекта: "
read -r choice

# Проверяем выбор
if ! [[ "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt "${#projects[@]}" ]; then
  echo "❌ Неверный номер."
  exit 1
fi

project="${projects[$((choice-1))]}"
echo "🚀 Сборка проекта: $project"

# Активируем среду
source ~/zephyrproject/.venv/bin/activate

# Запускаем west build
cd "$project"
west build --pristine=always -b nucleo_wl55jc

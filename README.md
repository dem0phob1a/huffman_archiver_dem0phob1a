# huffman_archiver

Консольный архиватор на базе алгоритма Хаффмана. Учебная практика первого
курса, кафедра системного программирования СПбГУ.

## Статус

Этап 1: базовая реализация (построение дерева перебором, без приоритетной
очереди), сжатие и распаковка произвольных файлов, модульные и
интеграционные тесты.

## Сборка

Требуется CMake >= 3.16 и компилятор с поддержкой C++17.

```sh
cmake -B build -S .
cmake --build build --parallel
```

Собрать с санитайзерами (ASan + UBSan):

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DHUFFMAN_ENABLE_SANITIZERS=ON
cmake --build build --parallel
```

## Использование

```sh
# сжать
./build/src/huffman_archiver -c input.txt output.huf

# распаковать
./build/src/huffman_archiver -d output.huf restored.txt
```

## Тесты

```sh
cd build
ctest --output-on-failure
```

## Формат архива

```
"HUF1"                      4 байта, магическое число
original_size                8 байт, little-endian, размер исходного файла
distinct_symbol_count        2 байта, little-endian
distinct_symbol_count раз:
    symbol                    1 байт
    frequency                 8 байт, little-endian
bitstream                   закодированные данные, дополненные нулями до целого числа байт
```

## Эксперименты

Подготовить входные наборы (путь к роману можно заменить своим):

```powershell
powershell -ExecutionPolicy Bypass -File .\prepare_test_data.ps1 `
    -SourceText .\experiments\data\text\voyna_i_mir.txt
```

Собрать Release-версию и установить зависимость для графиков:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
python -m pip install -r experiments/requirements.txt
```

Выполнить 30 замеров на каждом наборе:

```powershell
python experiments/run_benchmarks.py `
    --exe .\build\src\huffman_archiver.exe `
    --runs 30
```

Скрипт сохраняет все отдельные прогоны в `experiments/results/benchmark_runs.csv`,
среднее и выборочное стандартное отклонение по каждому набору — в
`benchmark_summary.csv`. Там же создаются метаданные окружения и графики
степени сжатия, времени сжатия и времени распаковки. Время включает запуск
процесса и файловый ввод-вывод. Размер полного текста зависит от переданного
источника; в текущем наборе он составляет 3,119,783 байта.

CLI-интеграционные тесты запускаются вместе с остальными через CTest:

```powershell
ctest --test-dir build --output-on-failure
```

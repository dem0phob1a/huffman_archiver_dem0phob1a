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

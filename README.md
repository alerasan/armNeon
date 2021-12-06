# armNeon

# Кратко
В python-версии 100 повторений алгоритма - 2.4 сек
В C-версии 100 повторений алгоритма - 0.856 сек
В C+NEON-версии 100 повторений алгоритма - 0.168 сек

В С+NEON-версии есть некоторые проблемы с точностью float-point. При агрессивной оптимизации в gcc используется другая модель работы с float-point, что приводит к расхождениям в единицу на двух итерациях. 

# Исходный алгоритм
Доступна python-версия алгоритма: https://github.com/alerasan/kwsAgc

# Железо
Проверялось на RPi4, на котором есть полноценная AArch64, в том числе полная поддержка NEON

Операционная система - обычная сборка Ubuntu 21

## Сборка
./build.sh
Создает директорию build/. Собирает две цели.

Первая цель сборки - обычный C-код, без использования NEON
Вторая цель сборки - C-код, использующий NEON Intrinsics

собранные экзешники появляются в директории build, agc_test и agc_test_neon соответственно

## Запуск
./run.sh
Простейший скрипт запуска, задает входные аргументы и выполняет программы (agc_test, agc_test_neon)
входные аргументы:
1. Файл с входными данными
2. Файл с результатами
3. Количество повторений 
4. флаг verbose

Количество повторений используется для некоего "взвешивания" оценки времени, чтобы минимизировать стохастику от запуска к запуску

## Выход выполнения
После выполнения выводится затраченное время, сравнение результата и разница в значениях (если она есть)


## Результат
Сейчас вывод выглядит вот так (100 повторений, нет вербализации):

### executing regular ###

I don't use NEON

PASSED

time taken for 100 repeats: 856.667ms

### executing neon ###

I was built with NEON usage

FAILED

time taken for 100 repeats: 168.329ms

FAILED в NEON-варианте происходит из-за агрессивной оптимизации операций с плавающей точкой.
Всего ошибки происходят на 2х итерациях и разница в единицу:

iteration: 43458 expected: 14184 received: 14183

iteration: 43533 expected: -14184 received: -14183

Я пробовал добавить флаги gcc с сохранением плавающей точки, но это не помогло. Тут, конечно, надо поисследовать глубже, что же конкретно происходит и почему происходит такое округление, но пока что это отложено на будущее.

## Дальнейшая работа
1. float-point ошибка - исследовать откуда берется и устранить
2. Рассмотреть возможность использования ассемблера вместо Intrinsics, это может дать бОльший буст производительности
3. Авто-тесты


## Hardware
was tested on RPi4, which has AArch64 (v8-A), including NEON

## Build
./build.sh
creates build/ directory, runs for two targets.
First target is regular C-code, without NEON usage
Second target is using NEON Intrinsics
builded executables appear at build folder, named agc_test and agc_test_neon

## Run
./run.sh
simple run script which sets input arguments and runs programs
arguments:
1. input file
2. output file
3. number of repeats
4. verbose

number of repeats were included to provide "weightened" estimation of time

## Output
after execution code prints elapsed time, assert result and  difference in values (if they are present)


## Summary
Right now it looks like this (100 repeats, no verbose):

### executing regular ###

I don't use NEON

PASSED

time taken for 100 repeats: 856.667ms

### executing neon ###

I was built with NEON usage

FAILED

time taken for 100 repeats: 168.329ms


FAILED in NEON variant happens because of agressive float-point optimizations

verbosed output shows exact difference in values:

iteration: 43458 expected: 14184 received: 14183

iteration: 43533 expected: -14184 received: -14183

so here are two iterations with fails and I assume this as future work

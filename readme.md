Инструкция по пользованию:
1) Установите маску расширений файлов, которые будут обработаны программой, правильная маска выглядит так (без кавычек): ".txt .bin .exe"
2) Выберите папку, в которой программа будет искать файлы для обработки. Будут обработаны все доступные в папке файлы, расширения которых соответствуют маске
3) Выберите тип сохранения: а) При перезаписи исходный файл будет модифицирован
б) При создании нового файла будет создан файл формата "исходноеИмя_output1" в папке назначения
4) Выберите папку для сохранения файлов
5) Установите нужные флажки
6) Введите число-модификатор, с которым будет проведена операция xor. Число ограничено по величине программно, оно автоматически сбросится если значение недопустимо
7) Нажмите "Старт" и ожидайте завершения работы программы. В окне статуса отобразится сообщение об окончании работы программы.
Программа работает в отдельном потоке, поэтому она не перестанет отвечать, однако интерфейсом программы во время её работы пользоваться не получится в целях сохранения работоспособности.

Компиляция:
```sh 
qmake xor_ported.pro
```

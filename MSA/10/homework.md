# 10. Основы работы с Docker

## Создать минимальный сервис, который

отвечает на порту 8000 <BR>
имеет http-метод: <BR>
GET /health/ <BR>
RESPONSE: {"status": "OK"} <BR>



**Запуск:**
```console
docker run -d -p 8000:8000 likerpic/server_c:1.0
```

Проверяем:
```console
$ curl localhost:8000/health
{"status" : "OK"}
```

```diff
+Работает!
```


Простой веб-сервер отвечает на метод GET по адресам:

`/` - возвращает обратно заголовок запроса <BR>
`/health` - возвращает `{"status" : "OK"}` ****
`/metrics` - возвращает метрику кол-во запросов (задел для Прометея) ****




**План действий:**
1. Скомпилировать на Си под alpine готовый исполняемый файл `server`
2. Подготовить `Dockerfile`, где среди прочего предусмотреть копирование `server` в рабочий каталог /app
3. Собрать image
4. Зарегистрироваться на Dcoker Hub.
5. Сделать `push` образа


Получившийся `Dockerfile`:
```console
FROM alpine:3.7


RUN mkdir /app
COPY server /app

WORKDIR /app/

ENTRYPOINT ["./server"]
CMD ["8000"]
```

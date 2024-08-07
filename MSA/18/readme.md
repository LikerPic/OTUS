# 18. Prometheus. Grafana

## Написать манифесты для деплоя в k8s для этого сервиса.

Инструментировать сервис из прошлого задания метриками в формате Prometheus с помощью библиотеки для вашего фреймворка и ЯП.

Сделать дашборд в Графане, в котором были бы метрики с разбивкой по API методам:
- Latency (response time) с квантилями по 0.5, 0.95, 0.99, max
- RPS
- Error Rate - количество 500ых ответов

Добавить в дашборд графики с метрикам в целом по сервису, взятые с nginx-ingress-controller:
- Latency (response time) с квантилями по 0.5, 0.95, 0.99, max
- RPS
- Error Rate - количество 500ых ответов

Настроить алертинг в графане на Error Rate и Latency.




## На выходе должны быть предоставлена

0) скриншоты дашборды с графиками в момент стресс-тестирования сервиса. Например, после 5-10 минут нагрузки.
1) json-дашборды.
<BR>

**Результат:**

0) Скриншот Dashboard<BR>
![Grafana Dashboard](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%201.%20Grafana%20Dashboard.png)<BR>

1) Dashboard Grafana: [Grafana Server CRUD Dashboard v3.json](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Grafana%20Server%20CRUD%20Dashboard%20v3.json)


**Запуск:**

**0.** Подготовка
```console
minikube start
helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
helm repo update
helm install stack prometheus-community/kube-prometheus-stack -f d:\Minikube\Helm\prometheus\prometheus.yaml 
```

Запускаем 4 отдельных терминала 
```console
minikube tunnel
minikube dashboard
kubectl port-forward service/prometheus-operated  9090 ##// Прокинем порт Прометея
kubectl port-forward service/stack-grafana  9000:80    ##// Пробросим порт Графаны
```

**1.** Ссылка на чарт: https://github.com/LikerPic/OTUS/tree/master/MSA/18/helm_chart<BR>
**2.** Копируем чарт себе в каталог `helm_chart`<BR>
**3.** Создаем/обновляем инсталляцию
```console
helm install server-crud helm_chart
helm upgrade server-crud helm_chart
```
5. Удаляем инсталляцию
```console
helm uninstall server-crud
```
<BR><BR><BR>

**Попутно:**
**1.** Адрес для входа на UI Прометея: `localhost:9090` => Status -> Targets
**2.** Адрес для входа на UI Grafana  `localhost:9000` (admin / prom-operator)
**3.** Мануал по настройке Прометей Оператора: https://github.com/prometheus-operator/kube-prometheus
**4.** Описание для настройки мониторинга ingress https://kubernetes.github.io/ingress-nginx/user-guide/monitoring/
**5.** Команды для подачи нагрузки:
```console
FOR /L %G IN (1,1,10000) DO curl arch.homework/health
FOR /L %G IN (1,1,10000) DO curl arch.homework/user/8
```
<BR><BR><BR>

**Наблюдения/грабли:**
**1.** Строки с метриками для Прометея в формате plain-text и должны заканчиваться символом `\n`. Перевод строки `\r\n` не принимает.
**2.** Страница `/metrics` с метриками, которую опрашивает Прометей. Надо иметь ввиду, что каждый под публикует свою такую страницу:<BR>
![Pic 0](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%200.%20Service%20metrics.png)<BR>
**3.** Вот так выглядит страница Targets в Прометей. Видны три пода нашего сервиса. Сам этот target как настройка подхватился автоматом из Сервис-Монитора<BR>
![Pic 2.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%202.%20Prometheus%20Targets2.png)<BR>
**4.** Замеры опроса каждого пода в отдельности. Видно, что каждый под опрашивается раз в 30 сек, но это происходит не одновременно:<BR>
![Pic 2.1.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%202.1.%20ServiceMonitor.%20Scrape%20periods%201.png)<BR>
**5.** Замеры опроса сервиса:<BR>
![Pic 2.2.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%202.1.%20ServiceMonitor.%20Scrape%20periods%202.png)<BR>
**6.** На самом деле, для задания интервала нужно использовать настройку Сервис-Монитора `interval`, а не `scrapeTimeout`<BR>
Если `interval` не задать, то по умолчанию принимается `30s`, что и произошло на лекции:
![Pic 3.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%203.%20Mistake.%20scrapeTimeout.png)<BR>
**7.** Одну из метрик назвал `resp_code`. Графана (и Прометей) на это выводит предупреждение несмотря на то, что перед метрикой есть служебная строка #TYPE, указывающая на тип `count`: <BR>
![Pic 4.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%204.%20Mistake.%20Metric%20name.png)<BR>
**8.** Картинка из Интернета для иллюстрации связки Прометей + Сервис-Монитор + Сервис + Pod<BR>
![Pic 5.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%205.%20Monitoring%20arch.png)<BR>
**9.** При отправке GET запроса из Opera на самом деле устанавливается 3 коннекта: по одному проходит сам запрос, а два оставшихся висят с таймаутом ~2 мин и если нажать F5, то по ним придет следующий запрос, далее поднимется еще один коннект на упреждение и так далее. Такое подвисание в коннекте сильно влияет на мониторинг. Так что тут важно как именно измерять время обработки запроса - в момент открытия коннекта или в момент получения самого запроса.
**10.** Opera: если зажать F5, то браузер начинает слать много запросов и в какой-то момент может разорвать коннект не дождавшись ответа целиком - проблема оказалась в том, что если отправку ответа разбивать на два вызова `write` (сначала шлем заголовки в сетевой сокет, а потом тело ответа), то браузер после первой порции разрывает соединение - удалось увидеть только с помощью WireShark. И что самое неприятное, когда вызов `write` пытается записать в сокет, по которому уже клиент разорвал соединение, то процесс со всеми потоками просто молча завершается - это не удалось полечить.
**11.** Chrome: также открывает коннекты на упреждение, но зажимание F5 не дает лавину повторных запросов.
**12.** Как ни странно, но при нагрузке с помощью `curl` (Win) увеличение количества одновременно запущенных консолей эффекта не дает (нагрузка не поднимается выше ~30 RPS).
**13.** Для вывода квантиля 0.5с использовал формулу: `histogram_quantile(0.5, sum(rate(dur_sec_bucket[1m])) by (le,api))"` (см. скрин выше).
На выходе получилось что-то не совсем вменяемое или просто не разобрался как это читать. Пока выглядит как нерабочее решение вывода гистограмм.<BR>
Предлагаемый вариант формулы из файла лекции `actions.txt`:
`histogram_quantile(0.95, sum by(le, uri) (rate(http_server_requests_seconds_bucket{uri=~"/api/.*"}[1m])))`




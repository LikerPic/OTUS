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
1. Адрес для входа на UI Прометея: `localhost:9090` => Status -> Targets
2. Адрес для входа на UI Grafana  `localhost:9000` (admin / prom-operator)
3. Мануал по настройке Прометей Оператора: https://github.com/prometheus-operator/kube-prometheus
4. Описание для настройки мониторинга ingress https://kubernetes.github.io/ingress-nginx/user-guide/monitoring/
5. Команды для подачи нагрузки:
```console
FOR /L %G IN (1,1,10000) DO curl arch.homework/health
FOR /L %G IN (1,1,10000) DO curl arch.homework/user/8
```
<BR><BR><BR>

**Наблюдения:**
![Pic 0](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%200.%20Service%20metrics.png)<BR>
![Pic 2.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%202.%20Prometheus%20Targets2.png)<BR>
![Pic 2.1.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%202.1.%20ServiceMonitor.%20Scrape%20periods%201.png)<BR>
![Pic 2.2.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%202.1.%20ServiceMonitor.%20Scrape%20periods%202.png)<BR>
![Pic 3.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%203.%20Mistake.%20scrapeTimeout.png)<BR>
![Pic 4.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%204.%20Mistake.%20Metric%20name.png)<BR>
![Pic 5.](https://github.com/LikerPic/OTUS/blob/master/MSA/18/Pic%205.%20Monitoring%20arch.png)<BR>







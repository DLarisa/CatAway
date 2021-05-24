# CatAway - Food and Water Dispenser
Simulator for a smart food and water dispenser, designed for cats.

## Dependencies
Built using a virtual machine with Ubuntu Server. </br></br>

### Pistache library
```
$ sudo add-apt-repository ppa:pistache+team/unstable
$ sudo apt update
$ sudo apt install libpistache-dev
```

### Mosquito (message broker that implements the MQTT protocol)
```
$ sudo apt-get install libmosquitto-dev
$ sudo apt-get install mosquitto
$ sudo apt-get install mosquitto-clients
```

### Compile and run
Compile with ```g++ -std=c++17 cataway.cpp -o cataway -lpistache -lcrypto -lpthread -lmosquitto```</br></br>
Start the server with ```./cataway```

## Tests
To introduce a setting, type ```curl -X POST http://localhost:8080/settings/add/<settingName>/<value>```</br></br>
To get the value of a setting, type ```curl -X POST http://localhost:8080/settings/<settingName>```

All options are listed bellow
```
curl -X POST http://localhost:8080/settings/add/<setting>/<value>
curl -X GET http://localhost:8080/settings/<setting>  (where setting is one of "weight", "age", "eatingSpeed", "feedingSchedule")
curl -X GET http://localhost:8080/recommendedFood
curl -X GET http://localhost:8080/getBreaks
curl -X GET http://localhost:8080/currentQuantity/<option> (where option is one of "water", "food")
curl -X GET http://localhost:8080/dispenserStatus
curl -X GET http://localhost:8080/fillWater
curl -X GET http://localhost:8080/lastRefresh
```

### Using Mosquitto
To print the values of all settings: ```mosquitto_sub -h localhost -t settings```

## Team
  - [Alecsandru Ciobanu](https://github.com/alecs99)
  - [Ana Maria Stegarescu](https://github.com/StegarescuAnaMaria)
  - [Bianca Furculesteanu](https://github.com/Bia103)
  - [Larisa Dumitrache](https://github.com/DLarisa)
  - [Mihaela Antal-Burlacu](https://github.com/mihaela-mab)
  - [Oana Mariana Ivan](https://github.com/Oana-Ivan)

## Analytical report
The analytical report can pe found [here](https://github.com/mihaela-mab/CatAway/blob/main/Raport%20de%20analiza.pdf).

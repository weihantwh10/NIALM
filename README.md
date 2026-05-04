# NIALM
NONINTRUSIVE APPLIANCE LOAD MONITORING SYSTEM with IoT

## Introduction

This project focuses on monitoring the electrical usage of household appliances without requiring direct connections to each device. It follows the concept of Non-Intrusive Appliance Load Monitoring (NIALM), where power consumption is analyzed from a single point of monitoring in the electrical system.

By integrating Machine Learning and IoT technology, the system enables smart home functionality, allowing real-time tracking, analysis, and identification of appliance usage through connected devices.

<img width="1076" height="541" alt="image" src="https://github.com/user-attachments/assets/3e8ec6da-0d42-46f0-b96d-15617c7001c9" />


## Project Overview

This project uses the PZEM-004T v3.0 sensor module to monitor electrical parameters in a household environment. An AC power supply delivers power through a Miniature Circuit Breaker (MCB) to selected appliances, including a house light, air conditioner, and washing machine.

The PZEM sensor is connected to the AC supply and works with a current transformer to measure the current flowing through the power line from the MCB. It captures key electrical parameters such as voltage, current, power, energy, frequency, and power factor.

## Data Acquisition

The sensor is interfaced with an ESP32 DEVKIT V1 microcontroller. Since the ESP32 operates on 5V DC, the 240V AC supply is stepped down using a transformer. The regulated 5V output powers the ESP32 via its VIN and Ground pins.

The PZEM sensor sends measurement data to the ESP32, where data acquisition takes place. In this project, 10 samples are collected every second. These samples are processed to compute the mean and standard deviation, resulting in one summarized data output per second.

## Data Logging and Processing

The processed data is exported to an Excel file in real time for logging purposes. The dataset is then converted into CSV format, making it easier to import into machine learning workflows.

Model training and validation are performed using Jupyter Notebook. The machine learning models are specifically trained to recognize the electrical signatures of the appliances used in this project.

## Communication and Visualization

The ESP32 supports WiFi connectivity, allowing it to transmit sensor data to a Node-RED web server using the MQTT protocol. MQTT acts as a broker to facilitate communication between devices.

On the Node-RED platform, incoming data is visualized in real time through graphs and dashboards. The trained machine learning model is also deployed on Node-RED, where it processes incoming data and identifies which appliance is currently consuming power.

## System Functionality

This setup demonstrates the workflow of a Non-Intrusive Appliance Load Monitoring (NIALM) system. By analyzing electrical data, the system can detect and classify appliance usage without requiring direct monitoring of each device.




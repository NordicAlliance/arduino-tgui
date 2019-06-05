# tgui
A lightweight GUI for Arduino Uno type of MCU with small TFT screen, primarily for showing sensor data.

## Motivation
There are numerous GUI libraries for embedded devices, however after using several of popular ones it's clear that most of them are designed for general purpose usages. In the scenarios that a user wants to gather and show data from various sensors, many of the features in other GUI libaries can be compromised in order to achieve faster speed on lower end devices, such as Arduino Uno.

## Priority
The design of the library considers a priority of goal, which is reflected on the design choices:
- Just enough to show sensor data. HMI is lower prioritized
- Limited number of graphical representations. Label, progress bar, running chart, and XY plot are the prioritized chioces
-  Easy to read code. It requires less coding skills to read and modify the library

## Demo

![alt text](./docs/example-1.png "Example 1")

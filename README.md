[200~# Papa's Threaderia: A multithread pizzeria simulation

## Project Description

This is a project created to utilize POSIX threads in C by simulating a pizzeria.

The program receives and handles multiple pizza orders by taking them through various stages, from the order submission and asynchronous electronic payment, until the delivery of the pizza to the customer's home.

## Project Files

* pizzeria.c contains the C code
* pizzeria.h contains various constants definitions that can be altered to change the behavior of some aspects of the restaurant simulation
* test-res.sh is a script that compiles and runs the program for 10 orders, seeding it with the number 1000

## Instructions

1. Be sure to have a recent `gcc` version on your machine, if not:
	* (For Windows users: ) [Here is a tutorial](https://dev.to/gamegods3/how-to-install-gcc-in-windows-10-the-easier-way-422j)
	* (For Linux users: ) Run `sudo apt-get install gcc` in your terminal
	* (For Mac users: ) Run `brew install gcc` in your terminal (requires [homebrew](https://brew.sh/))
2. In a terminal that is in the project directory, run the script with `./test-res.sh`.
3. Play with and change the constants on pizzeria.h to your liking for different behavior. You can also change the arguments passed to the program via `test-res.sh` (first argument is order number, second is seed for pseudorandom number generation).

## Authors

Made with ❤️  by Alex Papadopoulos and Katerina Mantaraki for 🎓


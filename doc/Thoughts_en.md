# Some thoughts from the author

Looking at the content and extension of this project you may think I am a young *do-it-yourself* enthusiast
with strong motivation on building my own steering wheel for sim-racing.

Well... you are wrong.

Nor I'am young nor I want a custom-made steering wheel.

*What? So, what is the point of this project?*

## Genesis

At some point in a conversation with my fellow sim-racing partners, the question about purchasing a new steering wheel arose.
No doubt most steering wheels in the market are high quality (this sentence does not include you, Fanatec), but the prices,
starting at 550 euros ($ 600, more or less) are too high for the average sim racer in Europe.

Believe it or not, I started this project to demonstrate whether those prices are justified or not (more on this later).

As a software engineer I already had some skills on digital circuits, but had no previous experience in Arduino,
nor in open hardware, nor in *do-it-yourself* projects.
Zero. None.

## Motivation

Despite such a senseless starting point, I found strong personal motivation to work hard on it, but has nothing to do with sim-racing:

- I'm committed to my profession enough as to lead by example:
  - I wanted to demonstrate that test-driven development works,
    even if you don't have a popular testing framework,
    even if your are not able to automate your test,
    even if hardware is involved,
    even if you don't have professional tools.
  - I wanted to demonstrate how transcendent good software documentation is.
  - I wanted to demonstrate that high-quality software takes **less** effort than shoddy work.
- I wanted to leverage my knowledge about GIT, configuration management and the GitHub platform.
- Arduino has been around in chit-chat talks for many years.
  It was time to learn something about it.
- It's nice to have a new hobby.

## Did you get any help ?

None. Zero.

I asked for help in a well-populated sim-racing community with no success.
I announced the project at even more populated sim-racing communities, but no attention.

To my personal opinion, sim-racing community *sucks*.

## Is a commercial steering wheel worth $ 600?

In short, **yes**.

You can build a DIY steering wheel for **half the price**,
but there is much more to do in a commercial work that takes money:

- Building many units is not the same as building a single unit. You need a factory and a supply chain.
- You need retailers.
- You need transportation and logistics.
- You need customer support.
- You need employees.
- You have to pay your taxes.
- Etc.

Sure you get to the point.

However, I can assure that electronics are not a significant part of the cost of a steering wheel.

## Why ESP32 ?

My first attempt at this project was based on [Arduino Nano 33](https://docs.arduino.cc/hardware/nano-33-ble/)
and the [Mbed operating system](https://os.mbed.com/).
Soon, this turned out to be a wrong decision, even when I achieved a working prototype based on USB connectivity.
The hardware capabilities of such a DevKit were below my expectations (especially the built-in ADC), but, worst of all,
developer support was very defective (as you can still see [here](https://github.com/arduino/ArduinoCore-mbed/issues/376)). I ended up stuck in a third-party software bug.

For the following reasons, I now focus on the ESP32 architecture:

- Way better hardware capabilities.
- Active support is available from both the [ESP32 forums](https://esp32.com/) and the
  [Arduino-ESP32 project](https://github.com/espressif/arduino-esp32/issues).
- Cheaper hardware.
- [Wide community acceptance](https://github.com/topics/esp32).
- There is a lot of information available on the [Internet](https://randomnerdtutorials.com/).
- More manufacturers to choose from.

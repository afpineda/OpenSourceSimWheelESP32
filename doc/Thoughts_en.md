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

## Can I have a telemetry display in my steering wheel ?

Of course you can, but not within this project (for now). Let me explain.

This firmware was designed from the start with telemetry in mind, however,
there is no way to send telemetry data to the firmware:

- There is no standard way to send telemetry data to any device. It's a pity, because it's not hard to do.
- I could develop a plugin for [SimHub](https://www.simhubdash.com/]), but I don't have the skills.
- Even if the firmware receives telemetry data, there are other concerns:
  - Is the ESP32 hardware powerful enough to display telemetry data in "real time"?
  - How to display it? gear and speed? gear and revs? speed and revs? ...
    The user would be constrained by the firmware itself.
  - Which display hardware to use?

And there is a better way: buy a display supported by *SimHub* and put it into your steering wheel along with this ESP32 hardware, as a separate device. This way you can display anything you like, even a TV show.

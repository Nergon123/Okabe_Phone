# Okabe Phone
[![Discord](https://img.shields.io/discord/1333879897860603905?label=Discord%20Server)](https://discord.gg/TdPYUj9fAj) ![GitHub last commit](https://img.shields.io/github/last-commit/Nergon123/Okabe_Phone) ![GitHub Repo stars](https://img.shields.io/github/stars/Nergon123/Okabe_Phone)

Okabe phone is my project that I'm making just because I bored...lmao.

The project itself is recreation of phone of character from [vn game "Steins;Gate"](https://steins-gate.fandom.com/wiki/Steins;Gate_(visual_novel)) - [Rintarou Okabe](https://steins-gate.fandom.com/wiki/Rintaro_Okabe)

It's based on ESP32 microcontroller. [LILYGO T4](https://lilygo.cc/products/t4) To be precise

## A little bit of history 


I started this project in July 2024 when I was playing around with unpacking resources of games... When I unpacked Steins;Gate I saw resources of phone and thought to myself.

> Imagine if someone recreates the actuall device from game it would be so cool...

I quick searched in GitHub "Okabe Phone", "Rintarou Phone", "Steins Gate Phone". And there was no result whatsoever.

So then I decided that I could do that, but I didn't really announce that anywhere ( With exception.I actually sent image of phone in [Science Adventure Discord Server](https://discord.com/invite/YBmZzfA) first time in [#showcase at 09/08/2024, 19:04 UTC](https://discord.com/channels/213420119034953729/453675152287203368/1271529614946074755) )

26/01/2025 I decided to make repository public.


## TODO

- [ ] Audio
	- [ ] Ringtones
	- [ ] Notifications
	- [ ] UI Sounds 
- [ ] Vibration
- [ ] Code stuff
	- [ ] More readable code
	- [ ] More optimised code 

- [ ] Themes (There was also other phones with same UI but other colors in Steins;Gate LBP)
- [ ] "Maybe"
	- [ ] Ability to Send E-mail (not sms)
	- [ ] Ability to read imageboards
	- [ ] Ability to write posts on imageboards
	- [ ] Very basic web browser (This thing have only 320KB of RAM, it would be almost impossible to render modern pages)
- [ ] 3D printed shell
- [ ] Properly designed PCB
 
## Features

- [x] Ability to Write SMS
- [x] Ability to Send SMS
- [x] Ability to Read SMS
- [x] Ability to Call
- [x] Ability to Recieve Calls
- [x] Ability to Change Wallpapers

## Known Problems

- (CRASH) Watchdog resets when call recieves at core 0 (Background AT Command process)
- This thing overall unstable, right now it have status "Proof of Concept"
- This thing probably cannot be used comercially since copyrighted material is being used(I'm not sure about this statement but probably it's true)


## Building Device

See [DEVBUILD.MD](./DEVBUILD.MD)


## Info for Developers/Potential Contributors

See [DEVINFO.MD](./DEVINFO.MD)


## Showcase

See [SHOWCASE.MD](./SHOWCASE.MD)


## Contact

If you any question you can ask them via
- Issues
- [E-Mail](mailto:nergon123@proton.me?subject=[OkabePhone]%20I%20have%20a%20question)
- [My personal discord server](https://discord.gg/TdPYUj9fAj) 
- Discord DM ( username @nergon )

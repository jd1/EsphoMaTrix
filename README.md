# EspHoMaTriX (ehmtx)
A simple DIY status display build with a flexible 8x32 RGB LED panel implemented with [esphome.io](https://esphome.io)

# Introduction

There are some "RGB-matrix" status displays/clocks out there, the commercial one from Lamtric and some very good d.i.y.-alternatives. 

- [LaMetric](https://lametric.com/en-US/) commercial ~ 199€
- [Awtrix](https://awtrixdocs.blueforcer.de/#/)
- [PixelIt](https://docs.bastelbunker.de/pixelit/)

The other d.i.y. solutions have their pros and cons. I tried both and used AwTrix for a long time. But the cons are so big (after my opinion) that i started an esphome.io variant targeted to an optimized homeassistant integration. The main reason, for me is the homeassistant integration!

## State

**First release!**

It is a working solution with the core funtionality coded. Advanced features, like automatic brigtness control can be done with esphome actions and automations. 

See it in action [youtube](https://www.youtube.com/watch?v=ZyaFj7ArIdY) (boring, no sound but subtitles)

## Features

Based a on a 8x32 RGB flexible matrix it displays a clock, the date and up to 16 other screens provided by home assistant. Each screen (value/text) can be associated with a 8x8 bit RGB icon or gif animation (see installation). The values/text can be updated or deleted from the display queue. Each screen has a lifetime, if not refreshed in its lifetime it will disapear.

### working sample

You can use the ehmtx32.yaml as sample for an ESP32. As mentioned you have to edit to your needs. So check font, icons, board and the GPIO port for your display.

The file ehmtx32.yaml uses the function ehmtx provides, the sample file ehmtx8266.yaml uses actions where possible.

# Installation

## Font
Download a small "pixel" TTF-font, i use ["monobit.ttf"](https://www.google.com/search?q=monobit.ttf). You can modify this font with [FontForge](https://fontforge.org/) and added **€** on base of a **E** and so on. Due to copyright i can't provide my modified version :-(.

```
font:
  - file: monobit.ttf
    id: EHMTX_font
    size: 16
    glyphs:  |
      !"%()+*=,-_.:°0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz€@
```

## icons/animations
Download and install all needed icons (.jpg/.png)/animations (.gif) under the "ehmtx"-key. All icons are automagically scaled to 8x8 on compile-time. 

```
emhtx:
  icons: 
    - file: icons/rocket.gif
      id: boot 
    - file: temperature.png
      id: temp 
    - file: garage.gif
      id: garage
```

Gifs are limited to 8 frames to limit the flash space. Thr first icon in your list is the fallback in case of an error.

All other solutions provide ready made icons, especialy lametric has a big database of [icons](https://developer.lametric.com/icons). Please check the copyright of the icons you use. The amount of icons is limited to 64 in the code and also by the flashspace and the RAM of your board.

The id of the icons is used later to configure the screens to display. So you should name them clever.

## esphome component

### local use

If you download the componets-folder from the repo an install it in your esphome you have more stable installation. But if there are new features you won't see them. If needed customize the yaml to your folder structure.

```
external_components:
   - source:
       type: local
       path: components # e.g. /config/esphome/components

```

### use from repo direct

Use the github repo as component. Esphome refreshes the external components "only" once a day, perhaps you have to refresh it manually.

```
external_components:
  - source:
      type: git
      url: https://github.com/lubeda/EsphoMaTrix
```

## YAML configuration

**Sample**
```
ehmtx:
  id: rgb328 # needed to reference the components in services and actions etc.
  show_clock: 6 
  show_screen: 8
  duration: 5
  display8x32: ehmtxdisplay
  time: EHMTX_clock
  font_id: EHMTX_font
  icons: 
    - file: sample.png  # use your icons/animations here
      id: boot 
    - file: celsius.png
      id: temp 
    - file: garage door.gif
      id: garage
```

_Configuration variables:_
**id (Required, ID):** Manually specify the ID used for code generation and in service definitions.

**show_clock (Optional, seconds):** duration to display the clock after this time the date is display until next "show_screen"

**show_screen (Optional, seconds):** duration to display a screen or a clock/date sequence, a long text will be scrolled at least two times 

**duration (Optional, minutes):** lifetime of a screen in minutes (default=5). If not updates a screen will be removed after ```duration``` minutes

**yoffset (Optional, pixel):** yoffset of the font, default -5 (see installation/font)

**display8x32 (required, ID):** ID of the addressable display

**time (required, ID):** ID of the time component

**font (required, ID):** ID of the font component

**scroll_intervall (Optional, ms):** the intervall in ms to scroll the text (default=80), should be a multiple of the ```update_interval``` from the display (default: 16ms)

**anim_intervall (Optional, ms):** the intervall in ms to display the next anim frame (default=192), should be a multiple of the ```update_interval``` from the display (default: 16ms)

## Usage without homeassistant

You can add screens locally and display data directly from any local sensor. See this sample:

```
sensor:
  - platform: bh1750
    id: sensorlx
    ...
    on_value:
      then:
        lambda: |-
          char text[30];
          sprintf(text,"Light: %2.1f lx", id(sensorlx).state);
          id(rgb8x32)->add_screen("sun", text, 5, false); // 5 Minutes, no alarm
```

Take care that the ```char text[30];``` has enough space to store the formated text. 

## local trigger

There is a trigger available to do some local magic. The trigger ```on_next_screen``` is triggered every time a new screen is displayed (so doesn't trigger on the clock display!!). In lambda's you can use two local string variables:

**x (Name of the icon, std::string):** value to use in lamba

**y (displayed text, std::string):** value to use in lamba

See the examples:

#### write some information to the logs

```
ehmtx:
  ....
  on_next_screen:
    lambda: |-
        ESP_LOGD("TriggerTest","Iconname: %s",x.c_str());
        ESP_LOGI("TriggerTest","Text: %s",y.c_str());
```

#### send event to homeassistant

To send data back to home assistant you can use events.

```
ehmtx:
  ....
  on_next_screen:
    - homeassistant.event:
      event: esphome.next_screen
      data_template:
          iconname: !lambda "return x.c_str();"
          text: !lambda "return y.c_str();"
```
### Actions

For local automations you can use actions. This is the normal way of automations. The ```id(rgb8x32)->``` style will also work.

#### Force screen

Force the selected screen ```icon_name``` to be displayed next. Afterwards the loop is continuing from this screen. e.g. helpfull for alarms. Or after an update of the value/text.

```
    - ehmtx.force.screen:
        id: rgb8x32
        icon_name: !lambda return icon_name;
```

#### Indicator on

The indicator is a static colored corner on the display.

You have to use use id of your ehmtx component, e.g. ```rgb8x32```

```
     - ehmtx.indicator.on:
        id: rgb8x32
        red: !lambda return r;
        green: !lambda return g;
        blue: !lambda return b;
```

- ```red, green, blue```: the color components (0..255) (default=80)

#### Indicator off 

```
     - ehmtx.indicator.off:
            id: rgb8x32
```

#### add screen to loop

```
        - ehmtx.add.screen:
            id: rgb8x32
            text: !lambda return text;
            icon_name: !lambda return icon_name;
            duration: 7
            alarm: false
```

Parameters:
**id (required, ID):** ID of the ehmtx component

**text (required, string):** the text to display

**icon_name (required, string):** the name of the icon to display

**duration (optional, int):** the lifetime of the screen in minutes (default=5)

**alarm (optional, bool):** if alarm set true (default=false)

## Integration in homeassistant

To control your display it has to be integrated in homeassistant. Then it provides at least three services, all prefixed with the devicename e.g. "ehmtx". See the sample yaml for the default services, but you can add your own.

### use the light component

To use the light component add the sample lambdas```on_turn_on``` and ```on_turn_off``` to the light component.

Sample:

```
light:
  - platform: neopixelbus
    id: ehmtx_light
    ....
    on_turn_on:
      lambda: |-
         id(rgb8x32)->set_enabled(false);
    on_turn_off:
       lambda: |-
         id(rgb8x32)->set_enabled(true);
```

### Services

All communication with homeassistant use the homeasistant-api. The services are defined in the yaml. To define the services you need the id of the ehmtx-component e.g. ```id(rgb8x32)```.

*Sample *
```
api:
  services:
    - service: alarm
      variables:
        icon_name: string
        text: string
      then:
        lambda: |-
          id(rgb8x32)->add_screen(icon_name, text, 7, true); // 7 Minutes alarm=true
```

Service **_brightness**

Sets the overall brightness of the display (0..255)

parameters:
- ```brightness```: from dark to bright (0..255) (default=80) as set in the light component by ```color_correct: [30%, 30%, 30%]```

Service **_alarm**

Sets an alarm, the alarm is like a normal screen but is displayed two minutes longer than a normal screen and has a red text color and a red marker in the upper right corner.

parameters:
- ```icon_name```: The name of the predefined icon-id (see installation)
- ```text```: The text to be displayed

Service **_screen**

Queues a screen with an icon/animation and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature you have to use different icons (ids)!
You can update the text on the fly. If the screen is displayed and you change the text for the icon it will start a new lifetime (see ```duration```) with the new text. 

parameters:
- ```icon_name``` The number of the predefined icons (see installation)
- ```text``` The text to be displayed

Service **_screen_t**

like above with a special duration. E.G. to indicate someones birthday you can use 24*60= 1440 minutes

parameters:
- ```icon``` The number of the predefined icons (see installation)
- ```text``` The text to be displayed
- ```duration``` The lifetime in minutes

Service **del_screen**

Removes a screen from the display by icon name.

parameters:
- ```icon_name``` The name of the icons as in the yaml (see installation)

Service **indicator_on**

Display a colored corner on all screens and the clock. You can define the color by parameter.

parameters:
- ```r``` red in 0..255
- ```g``` green in 0..255
- ```b``` blue in 0..255

Service **indicator_off**

removes the indicator

Service **status**

This service displays the running queue and a list of icons in the logs

```
[13:10:10][I][EHMTX:175]: status status: 1  as: 1
[13:10:10][I][EHMTX:176]: status screen count: 3
[13:10:10][I][EHMTX:181]: status slot: 0 icon: 36  text: 47.9°C end: 400
[13:10:10][I][EHMTX:181]: status slot: 1 icon: 23  text: Supa langer Text end: 310
[13:10:10][I][EHMTX:181]: status slot: 2 icon: 1  text: 10.3°C end: 363
[13:10:10][I][EHMTX:186]: status icon: 0 name: boot
[13:10:10][I][EHMTX:186]: status icon: 1 name: temp
[13:10:10][I][EHMTX:186]: status icon: 2 name: garage
[13:10:10][I][EHMTX:186]: status icon: 3 name: wind
[13:10:10][I][EHMTX:186]: status icon: 4 name: rain
```

### use in automations from homeassistant

The easiest way to use ehmtx as a status display is to use the icon names as trigger id. In my example i have an icon named "wind" when the sensor.wind_speed has a new state this automation sends the new data to the screen with the icon named "wind" and so on.

```
alias: EHMTX 8266 Test
description: ''
trigger:
  - platform: numeric_state
    entity_id: sensor.wind_speed
    id: wind
  - platform: state
    entity_id: sensor.actual_temperature
    id: temp
  - platform: state
    entity_id: sensor.wg_cover_device
    id: cover
condition: []
action:
  - service: esphome.ehmtx8266_screen
    data:
      icon_name: '{{trigger.id}}'
      text: >-
        {{trigger.to_state.state}}{{trigger.to_state.attributes.unit_of_measurement}}
mode: queued
max: 10
```

## Hardware/Wifi

Adapt all other data in the yaml to your needs, I use GPIO04/GPIO16 (esp8266/ESP32) as port for the display.

## Buzzer, Sound, buttons and automatic brightness

Awtrix and PixelIt have hardcoded functionality. EHMTX is also capable to build something like that by lambdas. But this is all your freedom.

E.G: an automatic brightness controll by an bh1570 sensor

```
sensor:
  - platform: bh1570
    # ...
    on_value:
      then:
         lambda: |-  
            if (x > 200) then
            {
               id(rgb8x32)->set_brightness(50);
            } else {
               id(rgb8x32)->set_brightness(250);
            }
```

# Usage

The integration works with the homeassistant api so, after boot of the device, it take some seconds until the service calls work.

# Disclaimer
THE SOFTWARE IS PROVIDED "AS IS", use at your own risk!

# Thanks

**[jd1](https://github.com/jd1)** for his contributions

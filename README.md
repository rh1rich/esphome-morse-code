# esphome-morse-code

This is an ESPHome component to send text as Morse code via a simple (binary) output.

## Idea

The idea behind this component was, to have the possibility to "play" a custom sound, like the `rtttl` component, but through a simple (binary) buzzer.

To simplify the definition of the beep sequence, after some consideration, I came up with the idea of using Morse code.

## Implementation details

The component provides the action `start` to translate a `text` into Morse code and send it through a binary output.

Each character is translated into short and long pulses. A short puls is called 'dit', a long one 'dah'. They are also called symbols. A 'dah' is 3 times longer then a 'dit'.
The pause between a pulse is as long as a 'dit'. The pause between characters is 3 times a 'dit'. The pause between words ist 7 times a 'dit'.

| Symbol | Length | e.g. |
| ------ | ------ | ---- |
| Dit |  | 50 ms |
| Dah | 3 * Dit | 150 ms |
| Symbol space | Dit | 50 ms |
| Character space | 3 * Dit | 150 ms |
| Word space | 7 * Dit | 350 ms |

For the translation of the text into Morse code the following table is used. Each character not in this table is ignored. A space, tab or new line character is interpreted as a word seperator.

| Char | Morse code || Char | Morse code |
| ---- | ---------- || ---- | ---------- |
| a A | ▄ ▄▄▄ || 1 | ▄ ▄▄▄ ▄▄▄ ▄▄▄ ▄▄▄ |
| b B | ▄▄▄ ▄ ▄ ▄ || 2 | ▄ ▄ ▄▄▄ ▄▄▄ ▄▄▄ |
| c C | ▄▄▄ ▄ ▄▄▄ ▄ || 3 | ▄ ▄ ▄ ▄▄▄ ▄▄▄ |
| d D | ▄▄▄ ▄ ▄ || 4 | ▄ ▄ ▄ ▄ ▄▄▄ |
| e E | ▄ || 5 | ▄ ▄ ▄ ▄ ▄ |
| f F | ▄ ▄ ▄▄▄ ▄ || 6 | ▄▄▄ ▄ ▄ ▄ ▄ |
| g G | ▄▄▄ ▄▄▄ ▄ || 7 | ▄▄▄ ▄▄▄ ▄ ▄ ▄ |
| h H | ▄ ▄ ▄ ▄ || 8 | ▄▄▄ ▄▄▄ ▄▄▄ ▄ ▄  
| i I | ▄ ▄ || 9 | ▄▄▄ ▄▄▄ ▄▄▄ ▄▄▄ ▄ |
| j J | ▄ ▄▄▄ ▄▄▄ ▄▄▄ || 0 | ▄▄▄ ▄▄▄ ▄▄▄ ▄▄▄ ▄▄▄ |
| k K | ▄▄▄ ▄ ▄▄▄ || ! | ▄▄▄ ▄ ▄▄▄ ▄ ▄▄▄ ▄▄▄ |
| l L | ▄ ▄▄▄ ▄ ▄ || " | ▄ ▄▄▄ ▄ ▄ ▄▄▄ ▄ |
| m M | ▄▄▄ ▄▄▄ || $ | ▄ ▄ ▄ ▄▄▄ ▄ ▄ ▄▄▄ |
| n N | ▄▄▄ ▄ || & | ▄ ▄▄▄ ▄ ▄ ▄ |
| o O | ▄▄▄ ▄▄▄ ▄▄▄ || ' | ▄ ▄▄▄ ▄▄▄ ▄▄▄ ▄▄▄ ▄ |
| p P | ▄ ▄▄▄ ▄▄▄ ▄ || ( | ▄▄▄ ▄ ▄▄▄ ▄▄▄ ▄ |
| q Q | ▄▄▄ ▄▄▄ ▄ ▄▄▄ || ) | ▄▄▄ ▄ ▄▄▄ ▄▄▄ ▄ ▄▄▄ |
| r R | ▄ ▄▄▄ ▄ || + | ▄ ▄▄▄ ▄ ▄▄▄ ▄ |
| s S | ▄ ▄ ▄ || , | ▄▄▄ ▄▄▄ ▄ ▄ ▄▄▄ ▄▄▄ |
| t T | ▄▄▄ || - | ▄▄▄ ▄ ▄ ▄ ▄ ▄▄▄ |
| u U | ▄ ▄ ▄▄▄ || . | ▄ ▄▄▄ ▄ ▄▄▄ ▄ ▄▄▄ |
| v V | ▄ ▄ ▄ ▄▄▄ || / | ▄▄▄ ▄ ▄ ▄▄▄ ▄ |
| w W | ▄ ▄▄▄ ▄▄▄ || : | ▄▄▄ ▄▄▄ ▄▄▄ ▄ ▄ ▄ |
| x X | ▄▄▄ ▄ ▄ ▄▄▄ || ; | ▄▄▄ ▄ ▄▄▄ ▄ ▄▄▄ ▄ |
| y Y | ▄▄▄ ▄ ▄▄▄ ▄▄▄ || = | ▄▄▄ ▄ ▄ ▄ ▄▄▄ |
| z Z | ▄▄▄ ▄▄▄ ▄ ▄ || ? | ▄ ▄ ▄▄▄ ▄▄▄ ▄ ▄ |
|  |  || @ | ▄ ▄▄▄ ▄▄▄ ▄ ▄▄▄ ▄ |
|  |  || _ | ▄ ▄ ▄▄▄ ▄▄▄ ▄ ▄▄▄ |

## Configuration example

The external component is implemented in the ESPHome configuration file as follows:

    external_components:
      - source:
          type: git
          url: https://github.com/rh1rich/esphome-morse-code

    api:
      actions:
        - action: play_morse_code
          variables:
            text_string: string
          then:
            - morse_code.start:
                text: !lambda 'return text_string;'

    output:
      - platform: gpio
          id: buzzer
          pin: GPIO09
    
    morse_code:
      output: buzzer
      dit_duration: 50

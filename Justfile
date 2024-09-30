set working-directory := '.'
set shell := ["powershell", "-c"]

run:
  gcc -o retrospectGame.com \
    -std=gnu99 -Wall -Wextra -DDOS -O3 -nostdlib -m16 -march=i386 -Wno-unused-function \
    -ffreestanding -fomit-frame-pointer -fwrapv -fno-strict-aliasing -fno-leading-underscore -fno-pic -fno-stack-protector \
    -fcf-protection=none -msoft-float \
    "-Wl,--nmagic,-static,-Tlink.x" \
    src/main.c src/keyboard_interrupt.s
  
  objcopy -O binary retrospectGame.com

  dosbox-x.exe -conf dosbox.conf -prerun retrospectGame.com
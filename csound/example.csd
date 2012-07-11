<CsoundSynthesizer>
; To run this example, copy a file called example_in.wav to this directory.

<CsOptions>
-W -d -b 512 -B 1024 -o example_out.wav
</CsOptions>

<CsInstruments>
sr = 44100
kr = 86.1328125
ksmps = 512
nchnls = 1
0dbfs = 1

instr 1
    ainput init 0
           fin "example.wav", 0, 1, ainput
    asig mmnotr ainput, 0.01, 0.1, 0
    out asig
endin
</CsInstruments>

<CsScore>
;    start   dur
i 1    0      5
e
</CsScore>

</CsoundSynthesizer>

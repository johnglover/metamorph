<CsoundSynthesizer>

<CsOptions>
-iadc0 -odac -b512 -B2048
</CsOptions>

<CsInstruments>
sr = 44100
kr = 86.1328125
ksmps = 512
nchnls = 1
0dbfs = 1

instr 1
    aIn inch 1
    aProcessed mm aIn, 0, 1, 0
    out aProcessed
endin
</CsInstruments>

<CsScore>
i1 0 60
e
</CsScore>

</CsoundSynthesizer>

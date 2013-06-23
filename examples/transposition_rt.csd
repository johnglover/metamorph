<CsoundSynthesizer>
; to run:
; csound transposition_rt.csd --omacro:transpose=<factor>

<CsOptions>
-iadc0 -odac -b512 -B2048
</CsOptions>

<CsInstruments>
sr = 44100
ksmps = 512
nchnls = 1
0dbfs = 1

instr transpose
    aIn inch 1
    aProcessed mm aIn, 1, 0, 0, 0, $transpose
    out aProcessed
endin
</CsInstruments>

<CsScore>
i "transpose" 0 60
</CsScore>

</CsoundSynthesizer>

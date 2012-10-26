<CsoundSynthesizer>
; to run:
; csound transposition.csd -i <input> -o <output> --omacro:transpose=<factor>

<CsOptions>
-W -d -b 512 -B 2048
</CsOptions>

<CsInstruments>
sr = 44100
kr = 86.1328125
ksmps = 512
nchnls = 1
0dbfs = 1

instr setup
	iDuration filelen "-i"
	event_i "i", "transpose", 0, iDuration
	turnoff
endin

instr transpose
    aIn inch 1
    aProcessed mm aIn, 1, 0, 0, 0, $transpose
    out aProcessed
endin
</CsInstruments>

<CsScore>
i "setup" 0 1
</CsScore>

</CsoundSynthesizer>

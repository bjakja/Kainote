ovText = "AviSynth Authors:"+chr(13)+
	\ "----------------------------"+chr(13)+
	\ "Ben Rudiak-Gould"+chr(13)+
	\ "Klaus Post"+chr(13)+
	\ "Richard Berg"+chr(13)+
	\ "Dividee"+chr(13)+
	\ "Ian Brabham"+chr(13)+
	\ "David Pierre"+chr(13)+chr(13)+
	\ "Documentation:"+chr(13)+
	\ "----------------------------"+chr(13)+
	\ "Wilbert Dijkhof"+chr(13)+
	\ "Jonathan Ernst"+chr(13)+
	\ "Ernst Peche"+chr(13)+
	\ "Alexander Balakhnin"+chr(13)+
	\ "Alex_e_Basta"+chr(13)+
	\ "Arne Stäcker"+chr(13)+chr(13)+
	\ "Contributions by:"+chr(13)+
	\ "----------------------------"+chr(13)+
	\ "Avery Lee, Milan, Naoki Shibata, "+chr(13)+
	\ "Peter Pawlowski, Olli Parviainen,"+chr(13)+
	\ "Nicolas Capens, DevIL team,"+chr(13)+
	\ "Donald Graft, poptones, ARDA"+chr(13)+
	\ "Kevin Atkinson, tritical, many more..."+chr(13)+chr(13)+
	\ "Thanks to:"+chr(13)+
	\ "----------------------------"+chr(13)+
	\ "Everyone contributing to the project,"+chr(13)+
	\ "Doom9 & team, kwag, stickboy"+chr(13)+
	\ ""


Blankclip(length=128, width=32,height=32, fps=25)
converttoyuy2()
Coloryuv(showyuv=true)
bg = loop(last + reverse(), 10)

c = framecount(bg)
t_mask = messageclip(ovText, height=c).converttoyv12().coloryuv(levels="tv->pc").trim(0,1)
t_blank = blankclip(t_mask, color=$ffffff)

t_mask_bl = t_mask.blur(1.0).blur(1.0).blur(1.0)

overlay(bg, t_mask_bl, mode="subtract")
overlay(t_blank, mode="blend", mask=t_mask)

frameevaluate("ol_x_offset = 40")
frameevaluate("ol_y_offset = 256 - (current_frame)")

fadeio(15)
converttoyuy2()

video = last

##############################################################
######################### AUDIO ##############################
##############################################################

s_type= "square"
freq_mul = 1.25
len_16 = 0.13

dis_16 = tone(len_16, 311/freq_mul, 44100, 1, s_type)   #   D#
cis_16 = tone(len_16, 277/freq_mul, 44100, 1, s_type)	#   C#
sil_16 = dis_16.amplify(0.0)				# Silence
e_16 = tone(len_16, 329.5/freq_mul, 44100, 1, s_type)	#   E
d_16 = tone(len_16, 293.7/freq_mul, 44100, 1, s_type)	#   D
c_16 = tone(len_16, 261.6/freq_mul, 44100, 1, s_type)	#   C

p1 = dis_16 ++ sil_16 ++ e_16 ++ dis_16 ++ dis_16 ++ e_16 ++ dis_16 ++ dis_16 ++ e_16 ++ dis_16 ++ dis_16 ++ dis_16 ++ d_16 ++ dis_16 ++ dis_16 ++ dis_16
p2= cis_16 ++ sil_16 ++ d_16 ++ cis_16 ++ cis_16 ++ d_16 ++ cis_16 ++ cis_16 ++ d_16 ++ cis_16 ++ cis_16 ++ cis_16 ++ c_16 ++ cis_16 ++ cis_16 ++ cis_16

p1 ++ p1 ++ p2 ++ p2
last ++ last ++ last
#convertaudioto16bit()
v_low = timestretch(pitch=50).amplifydb(-6)
v_high = timestretch(pitch=200).amplifydb(-12)
mergechannels(last, v_low, v_high)
converttomono()
amplifydb(-12)
audio = last

###################################################################

audiodub (video, audio)

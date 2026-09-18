	--[[
	Copyright (c) 2005-2010, Niels Martin Hansen, Rodrigo Braz Monteiro
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.
	* Neither the name of the Aegisub Group nor the names of its contributors
	may be used to endorse or promote products derived from this software
	without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
	]]

	util = require 'aegisub.util'
	ke4 = require 'effector-auto4'
	----------------------
	-- ke4 short functions
	R =		ke4.math.R
	Rs =	ke4.math.Rs
	Rd =	ke4.math.Rd
	Rc =	ke4.math.Rc
	Rm =	ke4.math.Rm
	Rds =	ke4.math.Rds
	Rsd =	ke4.math.Rds
	Rcs =	ke4.math.Rcs
	Rsc =	ke4.math.Rcs
	Rms =	ke4.math.Rms
	Rsm =	ke4.math.Rms
	Rr =	ke4.math.Rr
	Rsr =	ke4.math.Rsr
	Rrs =	ke4.math.Rsr
	Rdr =	ke4.math.Rdr
	Rcr =	ke4.math.Rcr
	Rmr =	ke4.math.Rmr
	Rdrs =	ke4.math.Rdrs
	Rcrs =	ke4.math.Rcrs
	Rmrs =	ke4.math.Rmrs
	Re =	ke4.math.Re
	----------------------
	
	table.copy = util.copy
	copy_line = util.copy
	table.copy_deep = util.deep_copy
	ass_color = util.ass_color
	ass_alpha = util.ass_alpha
	ass_style_color = util.ass_style_color
	extract_color = util.extract_color
	alpha_from_style = util.alpha_from_style
	color_from_style = util.color_from_style
	HSV_to_RGB = util.HSV_to_RGB
	HSL_to_RGB = util.HSL_to_RGB
	clamp = util.clamp
	interpolate = ke4.tag.ipol
	interpolate_alpha = ke4.tag.ipol
	interpolate_color = ke4.tag.ipol
	string.headtail = util.headtail
	string.trim = util.trim
	string.words = util.words

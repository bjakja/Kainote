--Script Usage: Make a Bezier curve in aeigsub using the clip drawing function.
--fixes by Bakura
--The Curve can not consist of more than one line.
--Example Line: "{\clip(m 0 300 b 0 0 300 0 300 300)}It really works!"

include("karaskel.lua")

script_name = "Bezier Curve Imposition"
script_description = "An effect to impose text onto a Bezier curve"
script_author = "an eer? & Bakura"
script_version = "1.0"

out={}
b_p = {}
b_t = {}
hasClip = false

function Bezier_impose(subs,sel)
    local meta, styles = karaskel.collect_head(subs)
	out = {}
	b_p = {}
	b_t = {}
	hasClip = false
	process_Bezier(subs, styles, meta, sel)
end


function do_fx(meta, line)
	local lastchar = ""
	local arc,g,u = 0,1,1
	if not hasClip then
		local clip = ""
		line.text = string.gsub(line.text, "\\i?clip%([^%)]+%)",
			function(clp)
				hasClip = true
				clip = clp
				return ""
			end
		)
		if not hasClip then
			error("Script needs clip with one Bezier curve to work.")
		end
		
		
		for v in clip:gmatch("%d+") do --Gets the Bezier points from the script. 
			b_p[u] = v
			u = u + 1
		end
	end

	local blockSplit = false
	local x = line.xL
	local y = line.yR
	arc = x
	local linetext = string.gsub(line.text, "\\frz?([0-9.-]+)", "")
	local linetags = ""
	
	for char in unicode.chars(linetext) do
		local split = (lastchar == "\\" and (char == "N" or char == "n" or char == "h"))
		if char == "{" then
			block = true
			linetags = linetags .. char
		elseif char == "}" then
			block = false
			linetags = linetags .. char
		elseif not block and not split and not blockSplit and char ~= " " then
			local width, height, descent, ext_lead = aegisub.text_extents(line.styleref, char)
			x = x + (width / 2)
			--dt is the small steps from one point to another, must be a small number. 
			local dt = .0001;

			--Define vars.
			local n,w,x_c,y_c = 0,1,{},{}

			--Creates a set of t to evaluate, 0 to 1 in steps of dt. 
			for w = 1,1/dt do
				b_t[w]=dt*w
			end

			--Finds the points of the Bezier curve for ever t.
			for ii = 1,#(b_t) do 
				x_c[ii] = (1-b_t[ii])^3*b_p[1] + 3*b_t[ii]*b_p[3]*(1-b_t[ii])^2+3*b_t[ii]^2*b_p[5]*(1-b_t[ii])+b_p[7]*b_t[ii]^3;
				y_c[ii] = (1-b_t[ii])^3*b_p[2] + 3*b_t[ii]*b_p[4]*(1-b_t[ii])^2+3*b_t[ii]^2*b_p[6]*(1-b_t[ii])+b_p[8]*b_t[ii]^3;
			end
			 
			--sums the arc lenght of the Bezier curve untill it reaches a letter.
			while n == 0 do
				g = g + 1
				if g > #x_c then
					error("Text width is larger than Bezier.\nMake larger Bezier or make text smaller")
				end
				arc = ((x_c[g]-x_c[g-1])^2+(y_c[g]-y_c[g-1])^2)^.5 + arc
				if x < arc then
					n = 1
				end
			end


			--The derivative of the Bezier cruve, used to find the slope of the Bezier curve.
			dx = 3*(-b_t[g]^2+2*b_t[g]-1)*b_p[1]+3*(3*b_t[g]^2-4*b_t[g]+1)*b_p[3]+3*(-3*b_t[g]^2+2*b_t[g])*b_p[5]+3*b_t[g]^2*b_p[7]
			dy = 3*(-b_t[g]^2+2*b_t[g]-1)*b_p[2]+3*(3*b_t[g]^2-4*b_t[g]+1)*b_p[4]+3*(-3*b_t[g]^2+2*b_t[g])*b_p[6]+3*b_t[g]^2*b_p[8]

			--Finds the angle of the slope.
			if dx < 0 then
				theta = math.pi+math.atan(dy/dx)
			else
				theta = math.atan(dy/dx)
			end

			--Creates lines.
			l = result()
			l.text = linetags .. string.format("{\\an2\\pos(%5.5f,%5.5f)\\frz%5.3f}%s", x_c[g], y_c[g], (-theta*(180/math.pi)), char)
			l.text = string.gsub(l.text, "}{", "")

			
			x = x + (width / 2)
			
		else 
			if not block then
				local width, height, descent, ext_lead = aegisub.text_extents(line.styleref, char)
				x = x + width
			end
				
			if block then
				linetags = linetags .. char
			end
		end
		lastchar = char
		blockSplit = split
	end

end

function process_Bezier(subs, styles, meta, selected)


	local commentPos = -1
	local i = 1
	local orgI = 1
	local selcounter = 1
	local checkSelections = true;

	while i <= #subs do
		aegisub.progress.task(string.format("Making gradient, line (%d / %d)", i, #subs))
		aegisub.progress.set((i - 1) / #subs * 100)
		if aegisub.progress.is_cancelled() then 
			aegisub.log("break")
			break 
		end
			
		local line = subs[i]
		if orgI == selected[selcounter] then
			if selcounter > #selected then
				break
				--aegisub.log("end counting %d", #selected)
			end
			--aegisub.log("sel counter %d next %d, %d, %d\n" .. line.text .. "\n", orgI, #selected, selected[selcounter], selcounter)
			selcounter = selcounter + 1
			isSelected = true
		else
			isSelected = false
		end
		orgI = orgI + 1

		if line.class == "dialogue" and not line.comment and isSelected then
		
			--aegisub.log("go to if")
			if commentPos < 0 then
				commentPos = i
			end
			
			function result()
				lc = copy_line(line);
				table.insert(out,lc)
				return lc
			end
			line.comment = true
			subs.insert(commentPos, line)
			line.comment = false
			i = i + 1
			commentPos = commentPos + 1
			if styles[line.style] then
				line.styleref = styles[line.style]
			else
				line.styleref = styles[1]
			end
			
			local marginr=(line.margin_r > 0) and line.margin_r or line.styleref.margin_r
			local marginl=(line.margin_l > 0) and line.margin_l or line.styleref.margin_l
			local margint=(line.margin_t > 0) and line.margin_t or line.styleref.margin_t
			local an = line.styleref.align
			local x = nil
			local y = nil
			local textwidth = 0
			local textheight = 0
			
			
			local linetext = ""
			linetext = line.text:gsub("\\an([0-9])",
			function(annum)
				an = tonumber(annum)
				return ""
			end)
			
			line.text = linetext:gsub("\\pos%(([0-9.-]+) ?, ?([0-9.-]+)%)",
			function(posx, posy)
				x = tonumber(posx)
				y = tonumber(posy)
				return ""
			end)
			
			linetext = line.text:gsub("\\move%(([0-9.-]+) ?, ?([0-9.-]+) ?, ?([0-9.-]+) ?, ?([0-9.-]+)%)",
			function(posx, posy, posx1, posy1)
				x = tonumber(posx)
				y = tonumber(posy)
				return ""
			end)
			line.text = linetext:gsub("\\move%(([0-9.-]+) ?, ?([0-9.-]+) ?, ?([0-9.-]+) ?, ?([0-9.-]+) ?, ?([0-9.-]+) ?, ?([0-9.-]+)%)",
			function(posx, posy, posx1, posy1, t1, t2)
				x = tonumber(posx)
				y = tonumber(posy)
				return ""
			end)
			
			textWithoutSplit = line.text
			hasDrawing = false
			drawingText = ""
			repeat
				a, b, p, toendbracket, rest = string.find(textWithoutSplit, "\\p([0-9]+)([^}]*)}(.*)")
				if p ~= nil then
					textWithoutSplit = rest
					local pnum = tonumber(p)
					if pnum > 0 then
						drawingText = drawingText .. rest
					else
						a, b, before = string.find(textWithoutSplit, "{(.-)\\p0")
						drawingText = string.sub(drawingText, 1, a)
					end
					hasDrawing = true
				end
			until p == nil
			
			local lineCount = 1
			local maxHeight = -1
			textWithoutSplit = string.gsub(textWithoutSplit, "\\(n|N)", 
				function(s)
					lineCount = lineCount + 1
					return "{\\" .. s .. "}"
				end)
				
			textWithoutSplit = string.gsub(textWithoutSplit, "\\h", " ")
				
			local maxTextWidth = 0
			local linestyle = copy_line(line.styleref)
			local prevstyle = copy_line(linestyle)
			local first = true
			for beforeText, tagsBlock in string.gmatch(textWithoutSplit .. "{@}", "(.-){([^{}]*)}") do 
				local isSplit = tagsBlock == "\\N" or tagsBlock == "\\n"
				if tagsBlock ~= "" and tagsBlock ~= "@" and not isSplit then 
					--linestyle = copy_line(line.styleref)
					for fs in string.gmatch(tagsBlock .. "\\", "\\fs([0-9.]+)") do 
						linestyle.fontsize = fs
					end
					for fscx in string.gmatch(tagsBlock .. "\\", "\\fscx([0-9.]+)") do 
						linestyle.scale_x = fscx
					end
					for fscy in string.gmatch(tagsBlock .. "\\", "\\fscy([0-9.]+)") do 
						linestyle.scale_y = fscy
					end
					for fn in string.gmatch(tagsBlock .. "\\", "\\fn([^\\}]+)") do 
						--aegisub.log("fn ".. fn .."\n")
						linestyle.fontname = fn
					end
					for fsp in string.gmatch(tagsBlock .. "\\", "\\fsp([0-9.-]+)") do 
						linestyle.spacing = fsp
					end
					if first then
						line.styleref = copy_line(linestyle)
					end
					--aegisub.log("text measuring st"..beforeText.." tb " .. tagsBlock .."\n")
				end 
				
				if beforeText ~= "" then
					--aegisub.log("text measuring "..beforeText.." fn " .. prevstyle.fontname .."\n")
					partLineWidth, partLineHeight, td, te = aegisub.text_extents(prevstyle, beforeText)
					textwidth = textwidth + partLineWidth
					if isSplit and maxTextWidth < textwidth then
						maxTextWidth = textwidth
						textwidth = 0
					end
					if maxHeight < partLineHeight then
						maxHeight = partLineHeight
					end
				end
				prevstyle = copy_line(linestyle)
			end
			
			if maxTextWidth > 0 then
				textwidth = maxTextWidth
			end
			if maxHeight > 0 then
				textheight = maxHeight
			end
			
			local drawingMinX = 0
			local drawingMinY = 0
			local drawingMaxX = 0
			local drawingMaxY = 0
			
			if hasDrawing then
				if drawingText == "" then
					hasDrawing = false
					aegisub.log("Line has empty drawing")
				else
					drawingMinX, drawingMinY, drawingMaxX, drawingMaxY = getVectorDrawingsBoundaries(drawingText)
					
					textwidth = textwidth + drawingMaxX - drawingMinX
					textheight = textheight + drawingMaxY - drawingMinY
					
					--aegisub.log("values %s, %s, %s, %s, %s, %s", drawingMinX, drawingMinY, drawingMaxX, drawingMaxY, textwidth, textheight)
					
				end
			end
			
			
			--orgx = 0
			--orgy = 0
			if x == nil then
				if an % 3 == 1 then
					x = marginl
					line.pos_x = x
				elseif an % 3 == 2 then
					x = (meta.res_x - textwidth) / 2
					line.pos_x = meta.res_x / 2
				else
					x = (meta.res_x - textwidth) - marginr
					line.pos_x = meta.res_x - marginr
				end

				if math.floor(an / 3.1) == 0 then
					y = meta.res_y - margint
					line.pos_y = y
				elseif math.floor(an/3.1) == 1 then
					y = (meta.res_y / 2) + (textheight / 2)
					line.pos_y = (meta.res_y / 2)
				elseif math.floor(an/3.1) == 2 then
					y = margint + textheight
					line.pos_y = margint
				end
			else
				line.pos_x = x
				line.pos_y = y
				if an % 3 == 2 then
					x = x - (textwidth / 2)
				elseif an % 3 == 0 then
					x = x - textwidth
				end
				if math.floor(an / 3.1) == 2 then
					y = y + textheight
				elseif math.floor(an / 3.1) == 1 then
					y = y + (textheight / 2)
				end
			end	
			
			if y ~= nil then
				if math.floor(an / 3.1) == 2 then
					drawingMaxY = drawingMaxY + y
					drawingMinY = drawingMinY + y
				elseif math.floor(an / 3.1) == 1 then
					drawingMaxY = drawingMaxY + (y - (textheight / 2))
					drawingMinY = drawingMinY + (y - (textheight / 2))
				else
					drawingMaxY = drawingMaxY + (y - textheight)
					drawingMinY = drawingMinY + (y - textheight)
				end
			end
					
			line.height = textheight
			line.y = y - (line.height / 2)
			line.i = i
			line.width = textwidth
			line.x = x + (line.width / 2)
			line.xL = hasDrawing and drawingMinX + x or x
			line.xR = hasDrawing and drawingMaxX + x or  line.xL + line.width
			line.yL = hasDrawing and drawingMinY or y - line.height
			line.yR = hasDrawing and drawingMaxY or y
			line.an = an
			line.has_drawing = hasDrawing
			line.text_stripped = line.text:gsub("\\{([^}}]*)}","")
			--aegisub.log("measures x %s, y %s, xL %s, yL %s, xR %s, yR %s", line.x, line.y, line.xL, line.yL, line.xR, line.yR)
			--out
			subs.delete(i)
			i = i - 1
			
			--aegisub.log("process gradient")
			do_fx(meta, line)
			--aegisub.log(string.format("process gradient out table %d", #out))
			if #out > 0 then

				for g = #out, 1, -1 do

					subs.insert(i + 1,out[g])

				end

				i = i + #out
				out = {}
			end
		end
		i = i + 1
	end
end

function getVectorDrawingsBoundaries(text)
	local drawing = text .. " "
	local minX = 10000000
	local minY = 10000000
	local maxX = -10000000
	local maxY = -10000000
	
	for i = 1, 10000000 do
		a, b, before, drawx, drawy, rest = string.find(drawing, "(.-)([0-9.-]+) ([0-9.-]+)(.*)")
		drawing = rest
		if drawx == nil or rest == nil then
			if rest ~= nil and string.match(rest, "[0-9.-]") ~= nil then
				error("Unpaired draw points\n")
				return nil
			end
			break
		end
		if string.match(before, "[0-9.-]") ~= nil then
			error("Bad drawing in: " .. before .. " " .. drawx .. " " .. drawy .. " " .. rest .. "\n")
			return nil
		end
		--aegisub.log("cl" .. drawx .. " " .. drawy .. " before " .. before .. "\n")
		local dx = tonumber(drawx)
		local dy = tonumber(drawy)
		if minX > dx then
			minX = dx
		end
		if minY > dy then
			minY = dy
		end
		if maxX < dx then
			maxX = dx
		end
		if maxY < dy then
			maxY = dy
		end
	end
	return minX, minY, maxX, maxY
end

aegisub.register_macro("Bezier", "Impose test onto a Bezier curve", Bezier_impose)

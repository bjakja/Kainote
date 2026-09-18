	--[[  La Librería effector-auto4.lua está diseñada con el fin de poder ampliar la gama de efectos
	hechos con Automation-auto4 del aegisub. Posee diversas funciones que ampliarán las posibilidades
	a la hora de desarrollar efectos karaokes, de traducción o de edición.
	Un gran porcentaje de las funciones del Kara Effector 3.6 se podrán usar en los templates auto-4,
	con unas ligeras modificaciones en la forma de usarlas, ya que éstas han tenido que ser adaptadas
	para que funcionen en Automation.
	
	--> Kara Effector 3.6 legacy
	--> Contáctanos:
	
	--> [WhatsApp] Kara Effector: +57 320 863 14 72
	--> [Discord]  Kara Effector: discord.gg/YFP2zeY
	--> http://www.facebook.com/karaeffector
	--> http://www.karaeffector.blogspot.com
	--]]
	
	-- effector-auto4.lua ---------------------------------------------------------------------------
	ke4_script_name		   = "effector-auto4.lua"
	ke4_script_description = "Librería de funciones del Kara Effector adaptadas para Automation-auto4"
	ke4_script_author	   = "Itachi Akatsuki"
	ke4_script_modified	   = "december 06th 2019; 13:34 (GMT + 5)"
	-------------------------------------------------------------------------------------------------
	
	--[[
	ke4 = {
		time = {
			HMS_to_ms( time_HMS )
			ms_to_HMS( time_ms )
			time_to_frame( Time )
			frame_to_ms( frames )
			frame_to_HMS( frames )
			mid1( val_i, val_n, Delay )
			mid2( val_i, val_n, Delay )
			li( val_i, val_n, Delay )
			lo( val_i, val_n, Delay )
			loop( j, maxj, Delay, Mode )
		}
		table = {
			ipol( Table, Size, Tags, algorithm )
			make( objet, size, limit_i, limit_f, ... )
			rmake( objet, size, limit_i, limit_f, ... )
			duplicate( Table )
			op( Table, mode, add )
			reverse( Table )
			view( Table, Table_name, indent )
			show( Table )
			compare( Table1, Table2 )
			inside( Table, e, str1, str2 )
			index( Table, e, str1, str2 )
			disorder( table_or_number )
			complete( Table, Start_time, End_time )
			filter( Table, Filter )
			replay( Len, ... )
			concat2( Table, ... )
			concat3( ... )
			concat4( ... )
			count( Table, e )
			pos( Table, e )
			string( String, Number_str )
			space( String )
			word( String )
			retire( Table, ... )
			combine( Table, n_combinations )
			inserttable( Table1, Table2, index_insert )
			cyclic( Table )
			random( Table_or_Number )
			delete( Table, ... )
			permute( Table )
			capture( String, Capture )
			gsub( Table, Capture, Replace )
			match( Table, Capture )
			unique( Table )
			copy( t, depth )
			tostring( t )
			gradient( Size, ... )
			type( Table )
		}
		utf8 = {
			charrange( s, i )
			chars( s )
			len( s )
		}
		math = {
			format( String, ... )
			R( Rand_i, Rand_f, Step )
			Rs( Rand_i, Rand_f, Step )
			Rd( Rand_i, Rand_f, Step )
			Rc( Rand_i, Rand_f, Step )
			Rm( Rand_i, Rand_f, Step )
			Rds( Rand_i, Rand_f, Step )
			Rcs( Rand_i, Rand_f, Step )
			Rms( Rand_i, Rand_f, Step )
			Rr( Rand_i, Rand_f, Step )
			Rsr( Rand_i, Rand_f, Step )
			Rdr( Rand_i, Rand_f, Step )
			Rcr( Rand_i, Rand_f, Step )
			Rmr( Rand_i, Rand_f, Step )
			Rdrs( Rand_i, Rand_f, Step )
			Rcrs( Rand_i, Rand_f, Step )
			Rmrs( Rand_i, Rand_f, Step )
			Re( Table )
			arc_curve( x, y, cx, cy, angle )
			angle( px1, py1, px2, py2 )
			create_matrix( ) = {
				get_data( )
				set_data( new_matrix )
				identity( )
				multiply( matrix2 )
				translate( x, y, z )
				scale( x, y, z )
				rotate( axis, angle )
				inverse( )
				transform( x, y, z, w )
			}
			degree( x1, y1, z1, x2, y2, z2 )
			polar( angle, radius, Return )
			distance( px1, py1, px2, py2 )
			distance2( x, y, z )
			intersect( x1, y1, x2, y2, x3, y3, x4, y4 )
			line_intersect( x0, y0, x1, y1, x2, y2, x3, y3, strict )
			factk( n )
			bernstein( i, n, t )
			confi_bezier( n, x, y, t, Return )
			length_bezier( ... )
			angle_bezier( points, t )
			point( n, x_range, y_range, start_x, start_y, end_x, end_y )
			ortho( x1, y1, z1, x2, y2, z2 )
			randomsteps( min, max, step )
			round( number_or_table, decimal )
			stretch( x, y, z, length )
			i( counter, A, B, C )
			circle( Shape )
			rotate( p, axis, angle )
			trim( x, min, max )
			matrix_sum( A, B )
			matrix_trans( A )
			matrix_esc( A, Escalar )
			matrix_mul( A, B )
			matrix_mul2( ... )
			matrix_cof( A, Return )
			matrix_det( A )
			matrix_adj( A )
			matrix_inv( A )
			matrix_dis( Disx, Disy )
			matrix_rot( Angle, Axis, Orgx, Orgy )
			matrix_rat( Ratx, Raty, Orgx, Orgy )
			matrix_ref( Mode )
			matrix_fil( Filxy, Axis )
			bezier( j, maxj, Return, ... )
			bezier2( pct, pts )
			to16( Num )
			clamp( Num, Min, Max )
			clamp2( Num, Min, Max )
			audio( Audio, Time, Start, Loops, Scale, Offset_time, Values )
		}
		random = {
			color( H, S, V )
			color2( H, S, V )
			colorvc( H, S, V )
			alpha( alpha_i, alpha_f )
			alphava( Ai, Af )
			e( ... )
			unique( table_or_number, index_r )
		}
		algorithm = {
			frames( starts, ends, dur )
			lines( text )
		}
		string = {
			count( String, Capture )
			toval( String )
			i( String )
			change( String, Capture, NoChange, NoCapture, Change )
			cap( String, Capture, Extra_Capture, Filter )
			coupling( String, Modified, Group )
			parts( String, Parts )
		}
		text = {
			to_shape( Text_Config, Text, Scale )
			bord_to_shape( Text_Config, Text, Scale, Bord )
			deformed( Text_Config, Text, Deformed, Pixel, Axis )
			deformed2( Text_Config, Text, Mode )
			to_clip( Text_Config, Text, relative_pos, iclip, Scale )
			to_pixels( Text_Config, Text, Ratio )
			bord_to_pixels( Text_Config, Text, Pixel )
			filter( Text_Config, Text, Split, ... )
			gradienth( Text_Config, Text, Relative_pos, ... )
			gradientv( Text_Config, Text, Relative_pos, ... )
			gradientangle( Text_Config, Text, Relative_pos, Angle, ... )
			bezier( Text_Config, Shape, Char_x, Char_y, Mode, Offset )
			rand( Text_Config, Text, num_tran, dur_tran, Tags, Table, Rand, All )
			upper( Text )
			lower( Text )
			kara( Text_Config )
			to_kara( Text, Kmode )
			inside( Text_Config, Count, Mark )
			outside( Text_Config, Count, Mark )
			tag( Text, ... )
			inclip( Text_Config, Center, Middle )
			outclip( Text_Config, Center, Middle )
			karaoke_true( Table )
			remove_tags( Text )
			remove_space_in_tags( Text )
			remove_extra_space( Text )
			remove_syls_nil( Text, Duration )
			to_word( Text, Duration )
			text2word( Text, Duration )
			text2syl( Text, Duration )
			text2char( Text, Duration )
			text2part( Text_Config, Text, Duration, Text_left, Parts )
			syl2hiragana( Text )
			syl2katakana( Text )
			kana2romaji( Text )
			text2stripped( Text )
			char2byte( Text )
			byte2char( Bytes )
		}
		shape = {
			ASSDraw3( Shape, Round )
			round( Shape, Round )
			info( Shape )
			redraw( Shape, tract, Section )
			modify( Shape, Modify )
			filter( Shape, Filter )
			filter2( Shape, Filter, Split )
			filter3( Shape, Split, ... )
			length( Shape, parts )
			width( Shape, Height )
			height( Shape )
			rotate( Shape, Angle, org_x, org_y )
			reflect( Shape, Axis, Relative )
			oblique( Shape, Pixel, Axis )
			to_bezier( Shape )
			reverse( Shape )
			origin( Shape )
			displace( Shape, Dx, Dy )
			incenter( Shape )
			centerpos( Shape, CenterX, CenterY )
			firstpos( Shape, pos_x, pos_y )
			ratio( Shape, Ratiox, Ratioy, Mode )
			size( Shape, SizeX, SizeY, Mode )
			divide( Shape, Mark )
			to_shape( Table_points )
			retire( Shape, Index_1, Index_2 )
			array( Shape, Loops, Angles_or_mode, Dxy )
			trajectory( Loop_t, distance_nim, distance_max )
			Ltrajectory( length_total, length_curve, height_curve )
			Ctrajectory( Loop_Ct, radius_min, radius_max )
			Rtrajectory( Loop_Rt, radius_min, radius_max )
			Strajectory( Loops_St, Radius )
			multi1( Size_shape, Px )
			multi2( Width, Height, Pixel )
			multi3( Size, Bord, Shape )
			multi4( Size, Loop1, Loop2, n )
			multi5( Shapes, Width, Height, Dxy )
			multi6( Size, Bord, Part )
			multi7( Part, Radius )
			multi8( Shape, Size_ini, Size_fin, Loop )
			multi9( Shape, Loop, Tags, Vertical )
			bord( Shape, Bord, Size )
			equality( Shape1, Shape2 )
			morphism( Size, Shape1, Shape2, Accel )
			to_pixels( Shape )
			point( Shape, Pixel )
			bord_to_pixels( Shape, Pixel )
			fxline( P1, P2, Radius )
			fxcircle( Shape )
			trim( Shape, Lines, Mark, Ratio )
			reduce( Shape )
			matrix( Shape, ... )
			iso( Shape1, Shape2, Trim )
			do_shape( Shape1, Shape2, Mode, Split )
			to_outline( Shape, Bord )
			deformed( Shape, Deformed, Pixel, Axis )
			deformed2( Shape, Defor_x, Defor_y )
			fusion( Shapes, Tags )
			filtershape( Shape, ... )
			from_audio( Audio, Time, Start, Width, Height, Thickness )
			bounding( shape )
			detect( width, height, data, compare_func )
			filtery( shape, filter )
			flatten( shape, curve_tolerance )
			glue( src_shape, dst_shape, transform_callback )
			move( shape, x, y )
			split( shape, max_len )
			to_outline2( shape, width_xy, width_y, mode )
			to_pixels2( shape )
			transform( shape, matrix )
			to_clip( Shape, Pos_x, Pos_y, Ratio, Iclip )
			intersect( Shape1, Shape2 )
			insert( Shape1, Shape2 )
		}
		ass = {
			convert_time( ass_ms )
			convert_coloralpha( ass_r_a, g, b, a )
			interpolate_coloralpha( pct, ... )
			create_parser( ass_text ) = {
				parse_line( line )
				meta( )
				styles( )
				dialogs( extended )
			}
			gradient( Size, Valor, ... )
			gradient_line( Text, ... )
			linefx( Table_Line, Parts, Complete, Real )
		}
		tag = {
			oscill( DurTotal, DurDelay, ... )
			set( times_set, events_set, Line_start_time, Line_end_time )
			only( condition, s_true, s_false )
			only2( Conditions, ... )
			movevc( Shape, posx, posy, Dx, Dy, Time_i, Time_f )
			movevci( Shape, posx, posy, Dx, Dy, Time_i, Time_f )
			glitter( DurTotal, ExtraTags_i, ExtraTags_f )
			glitterx( DurTotal, ExtraTags_i, ExtraTags_f )
			glittery( DurTotal, ExtraTags_i, ExtraTags_f )
			clip_shape( Shapes, Center_x, Center_y )
			ipol( Ipol_i, ... )
			clip( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
			iclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
			clip2( Left, Top, Width, Height )
			iclip2( Left, Top, Width, Height )
			rclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
			riclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
			moveclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
			moveiclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
			moveclip2( Left, Top, Width, Height, Move_x, Move_y, Time_i, Time_f )
			moveiclip2( Left, Top, Width, Height, Move_x, Move_y, Time_i, Time_f )
			moverclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
			movericlip( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
			operation( String )
			lmove( Configs, Coor, Times, Times2 )
			pmove( Configs, F_x, F_y, domainF, t1, t2, Accel, offset_t )
			smove( Configs, Shape, t1, t2, Relative )
			rmove( Configs, Rx, Ry, t1, t2, Accel, offset_t, Counter2 )
			rmove2( Configs, Rx, Ry, t, Accel )
			rmove3( Configs, Rx, Ry, t, Accel, offset_t )
			rmove4( Configs, Rx, Ry, t1, t2, Accel, offset_t, move4 )
			omove( Configs, P, t1, t2, Dur, Accel )
			default( Text_Config, String )
			default2( Text_Config, String )
			inverse( Text_Config, String )
			redefine( Text_Config, String )
			dark( Text_Config, String )
			HTML_to_ass( Text_Config, String )
			timefx( Text_Config, String )
			modify( Text_Config, String )
			do_alpha( String )
			tonumber( String )
			cyclic( j, maxj, Dur, Dur_tr, Delay, Fad_i, Fad_f, tags_ini, tags_fin )
			sec( j, maxj, Dur, Dur_tr, tags_ini, tags_fin )
		}
		color = {
			ass( html_color )
			ass2( Rnum, Gnum, Bnum )
			ass3( Hnum, Snum, Vnum )
			rgb( Color_or_Table, Matrix13, Multi )
			hsv( Color_or_Table, Matrix13, Multi )
			assF( Color_or_Table )
			to_RGB( Color_or_Table )
			to_HSV( Color_or_Table )
			vc( Color_or_Table )
			r( )
			rc( colorRC, ... )
			rvc( colorRVC, ... )
			gradientv( ColorTop_or_Table, ColorBottom_or_Table )
			gradienth( ColorLeft_or_Table, ColorRight_or_Table, val_i, val_n, algorithm )
			vc_to_c( colorvc_or_table )
			c_to_vc( colorc_or_table )
			interpolate( Color1_or_Table, Color2_or_Table, Index_Ipol )
			vector( Color1, Color2 )
			delay( time_i, delay, color_i, color_f, ... )
			movedelay( dur, delay, mode, ... )
			set( Times, Colors, Line_start_time, Line_end_time, ... )
			mask( Mode, Color, Mask )
			movemask( Dur, Delay, Mode, Color, Mask, val_i )
			masktable( Color_or_Table, Size, val_i )
			ipol( val_i, val_n, ... )
			loop( j, maxj, ... )
			bigradient( Color1_or_Table, Color2_or_Table, Size_Table, val_i )
			from_error( Color_or_Table )
			to_HTML( ASScolor )
			matrix( Color_or_Table, ... )
			colorchange( Color_or_Table, dur )
			fromstyle( ColorAlpha )
			val2ass( val_R, val_G, val_B )
			ipolfx( Ipol, Color1, Color2 )
			HSV_to_RGB( Hue, Saturation, Value )
		}
		alpha = {
			from_error( Alpha_or_Table )
			assF( Alpha_or_Table )
			va( Alpha_or_Table )
			r( )
			ra( ArA_alpha, ... )
			rva( ArCA_alpha, ... )
			gradientv( AlphaTop_or_table, AlphaBottom_or_table )
			gradienth( AlphaLeft_or_table, AlphaRight_or_table, val_i, val_n, algorithm )
			va_to_a( Alphava_or_Table )
			a_to_va( Alphaa_or_Table )
			interpolate( Alpha1_or_Table, Alpha2_or_Table, Index_Ipol )
			delay( time_i, delay, alpha_i, alpha_f, ... )
			mask( Mode, Alpha, Mask )
			ipol( Size, ... )
			loop( j, maxj, ... )
			bigradient( Alpha1_or_Table, Alpha2_or_Table, Size_Table, val_i )
			fromstyle( ColorAlpha )
			val2ass( val_A )
			ipolfx( Ipol, Alpha1, Alpha2 )
		}
		file = {
			get_lines( File_lua, Number_or_match )
			gsub( File_lua, ... )
			match( File_lua, Match_or_tbl )
			gmatch( File_lua, Match_or_tbl )
			count( File_lua, Match_or_tbl )
		}
		image = {
			data( bmp_image )
			to_pixels( bmp_image, Size, Return_pos )
		}
		decode = {
			create_bmp_reader( filename ) = {
				file_size( )
				width( )
				height( )
				bit_depth( )
				data_size( )
				row_size( )
				bottom_up( )
				data_raw( )
				data_packed( )
				data_text( )
			}
			create_wav_reader( filename ) = {
				file_size( )
				channels_number( )
				sample_rate( )
				byte_rate( )
				block_align( )
				bits_per_sample( )
				samples_per_channel( )
				min_max_amplitude( )
				sample_from_ms( ms )
				ms_from_sample( sample )
				position( pos )
				samples_interlaced( n )
				samples( n )
			}
			create_frequency_analyzer( samples, sample_rate )
				frequencies( )
				frequency_weight( freq )
				frequency_range_weight( freq_min, freq_max )
			create_font( family, bold, italic, underline, strikeout, size, xscale, yscale, hspace ) = {
				metrics( )
				text_extents( text )
				text_to_shape( text )
			}
			list_fonts( with_filenames )
		}
	}
	--]]

	-- Configuration
	local FP_PRECISION = 3			-- Floating point precision by numbers behind point (for shape points)
	local CURVE_TOLERANCE = 1		-- Angle in degree to define a curve as flat
	local MAX_CIRCUMFERENCE = 1.5	-- Circumference step size to create round edges out of lines
	local MITER_LIMIT = 200			-- Maximal length of a miter join
	local SUPERSAMPLING = 8			-- Anti-aliasing precision for shape to pixels conversion
	local FONT_PRECISION = 64		-- Font scale for better precision output from native font system
	local LIBASS_FONTHACK = true	-- Scale font data to fontsize? (no effect on windows)
	local LIBPNG_PATH = "libpng"	-- libpng dynamic library location or shortcut (for system library loading function)

	local ffix = require( "ffi" )
	local advapix, pangocairo, fontconfig
	if ffix.os == "Windows" then
		advapix = ffix.load( "Advapi32" )
		ffix.cdef( [[
		enum{CP_UTF8X = 65001};
		enum{MM_TEXT2 = 1};
		enum{TRANSPARENT2 = 1};
		enum{
			FW_NORMAL2 = 400,
			FW_BOLD2 = 700
		};
		enum{DEFAULT_CHARSET2 = 1};
		enum{OUT_TT_PRECIS2 = 4};
		enum{CLIP_DEFAULT_PRECIS2 = 0};
		enum{ANTIALIASED_QUALITY2 = 4};
		enum{DEFAULT_PITCH2 = 0x0};
		enum{FF_DONTCARE2 = 0x0};
		enum{
			PT_MOVETO2 = 0x6,
			PT_LINETO2 = 0x2,
			PT_BEZIERTO2 = 0x4,
			PT_CLOSEFIGURE2 = 0x1
		};
		typedef unsigned int UINT;
		typedef unsigned long DWORD;
		typedef DWORD* LPDWORD;
		typedef const char* LPCSTR;
		typedef const wchar_t* LPCWSTR;
		typedef wchar_t* LPWSTR;
		typedef char* LPSTR;
		typedef void* HANDLE;
		typedef HANDLE HDC;
		typedef int BOOL;
		typedef BOOL* LPBOOL;
		typedef unsigned int size_t;
		typedef HANDLE HFONT;
		typedef HANDLE HGDIOBJ;
		typedef long LONG;
		typedef wchar_t WCHAR;
		typedef unsigned char BYTE;
		typedef BYTE* LPBYTE;
		typedef int INT;
		typedef long LPARAM;
		static const int LF_FACESIZE2 = 32;
		static const int LF_FULLFACESIZE2 = 64;
		typedef struct{
			LONG tmHeight;
			LONG tmAscent;
			LONG tmDescent;
			LONG tmInternalLeading;
			LONG tmExternalLeading;
			LONG tmAveCharWidth;
			LONG tmMaxCharWidth;
			LONG tmWeight;
			LONG tmOverhang;
			LONG tmDigitizedAspectX;
			LONG tmDigitizedAspectY;
			WCHAR tmFirstChar;
			WCHAR tmLastChar;
			WCHAR tmDefaultChar;
			WCHAR tmBreakChar;
			BYTE tmItalic;
			BYTE tmUnderlined;
			BYTE tmStruckOut;
			BYTE tmPitchAndFamily;
			BYTE tmCharSet;
		}TEXTMETRICW, *LPTEXTMETRICW;
		typedef struct{
			LONG cx;
			LONG cy;
		}SIZE, *LPSIZE;
		typedef struct{
			LONG left;
			LONG top;
			LONG right;
			LONG bottom;
		}RECT;
		typedef const RECT* LPCRECT;
		typedef struct{
			LONG x;
			LONG y;
		}POINT, *LPPOINT;
		typedef struct{
		  LONG  lfHeight;
		  LONG  lfWidth;
		  LONG  lfEscapement;
		  LONG  lfOrientation;
		  LONG  lfWeight;
		  BYTE  lfItalic;
		  BYTE  lfUnderline;
		  BYTE  lfStrikeOut;
		  BYTE  lfCharSet;
		  BYTE  lfOutPrecision;
		  BYTE  lfClipPrecision;
		  BYTE  lfQuality;
		  BYTE  lfPitchAndFamily;
		  WCHAR lfFaceName[LF_FACESIZE2];
		}LOGFONTW, *LPLOGFONTW;
		typedef struct{
		  LOGFONTW elfLogFont;
		  WCHAR   elfFullName[LF_FULLFACESIZE2];
		  WCHAR   elfStyle[LF_FACESIZE2];
		  WCHAR   elfScript[LF_FACESIZE2];
		}ENUMLOGFONTEXW, *LPENUMLOGFONTEXW;
		enum{
			FONTTYPE_RASTER2 = 1,
			FONTTYPE_DEVICE2 = 2,
			FONTTYPE_TRUETYPE2 = 4
		};
		typedef int (__stdcall *FONTENUMPROC)(const ENUMLOGFONTEXW*, const void*, DWORD, LPARAM);
		enum{ERROR_SUCCESS2 = 0};
		typedef HANDLE HKEY;
		typedef HKEY* PHKEY;
		enum{HKEY_LOCAL_MACHINE2 = 0x80000002};
		typedef enum{KEY_READ2 = 0x20019}REGSAM;

		int MultiByteToWideChar(UINT, DWORD, LPCSTR, int, LPWSTR, int);
		int WideCharToMultiByte(UINT, DWORD, LPCWSTR, int, LPSTR, int, LPCSTR, LPBOOL);
		HDC CreateCompatibleDC(HDC);
		BOOL DeleteDC(HDC);
		int SetMapMode(HDC, int);
		int SetBkMode(HDC, int);
		size_t wcslen(const wchar_t*);
		HFONT CreateFontW(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCWSTR);
		HGDIOBJ SelectObject(HDC, HGDIOBJ);
		BOOL DeleteObject(HGDIOBJ);
		BOOL GetTextMetricsW(HDC, LPTEXTMETRICW);
		BOOL GetTextExtentPoint32W(HDC, LPCWSTR, int, LPSIZE);
		BOOL BeginPath(HDC);
		BOOL ExtTextOutW(HDC, int, int, UINT, LPCRECT, LPCWSTR, UINT, const INT*);
		BOOL EndPath(HDC);
		int GetPath(HDC, LPPOINT, LPBYTE, int);
		BOOL AbortPath(HDC);
		int EnumFontFamiliesExW(HDC, LPLOGFONTW, FONTENUMPROC, LPARAM, DWORD);
		LONG RegOpenKeyExA(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
		LONG RegCloseKey(HKEY);
		LONG RegEnumValueW(HKEY, DWORD, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
		]] )
	else
		pcall( function( )
			pangocairo = ffix.load( "pangocairo-1.0.so" )
			ffix.cdef( [[
			typedef enum{
				CAIRO_FORMAT_INVALID2   = -1,
				CAIRO_FORMAT_ARGB32X    = 0,
				CAIRO_FORMAT_RGB24X     = 1,
				CAIRO_FORMAT_A8X        = 2,
				CAIRO_FORMAT_A1X        = 3,
				CAIRO_FORMAT_RGB16_565X = 4,
				CAIRO_FORMAT_RGB30X     = 5
			}cairo_format_t;
			typedef void cairo_surface_t;
			typedef void cairo_t;
			typedef void PangoLayout;
			typedef void* gpointer;
			static const int PANGO_SCALE2 = 1024;
			typedef void PangoFontDescription;
			typedef enum{
				PANGO_WEIGHT_THIN2	= 100,
				PANGO_WEIGHT_ULTRALIGHT2 = 200,
				PANGO_WEIGHT_LIGHT2 = 300,
				PANGO_WEIGHT_NORMAL2 = 400,
				PANGO_WEIGHT_MEDIUM2 = 500,
				PANGO_WEIGHT_SEMIBOLD2 = 600,
				PANGO_WEIGHT_BOLD2 = 700,
				PANGO_WEIGHT_ULTRABOLD2 = 800,
				PANGO_WEIGHT_HEAVY2 = 900,
				PANGO_WEIGHT_ULTRAHEAVY2 = 1000
			}PangoWeight;
			typedef enum{
				PANGO_STYLE_NORMAL,
				PANGO_STYLE_OBLIQUE,
				PANGO_STYLE_ITALIC
			}PangoStyle;
			typedef void PangoAttrList;
			typedef void PangoAttribute;
			typedef enum{
				PANGO_UNDERLINE_NONE,
				PANGO_UNDERLINE_SINGLE,
				PANGO_UNDERLINE_DOUBLE,
				PANGO_UNDERLINE_LOW,
				PANGO_UNDERLINE_ERROR
			}PangoUnderline;
			typedef int gint;
			typedef gint gboolean;
			typedef void PangoContext;
			typedef unsigned int guint;
			typedef struct{
				guint ref_count;
				int ascent;
				int descent;
				int approximate_char_width;
				int approximate_digit_width;
				int underline_position;
				int underline_thickness;
				int strikethrough_position;
				int strikethrough_thickness;
			}PangoFontMetrics;
			typedef void PangoLanguage;
			typedef struct{
				int x;
				int y;
				int width;
				int height;
			}PangoRectangle;
			typedef enum{
				CAIRO_STATUS_SUCCESS2 = 0
			}cairo_status_t;
			typedef enum{
				CAIRO_PATH_MOVE_TO,
				CAIRO_PATH_LINE_TO,
				CAIRO_PATH_CURVE_TO,
				CAIRO_PATH_CLOSE_PATH
			}cairo_path_data_type_t;
			typedef union{
				struct{
					cairo_path_data_type_t type;
					int length;
				}header;
				struct{
					double x, y;
				}point;
			}cairo_path_data_t;
			typedef struct{
				cairo_status_t status;
				cairo_path_data_t* data;
				int num_data;
			}cairo_path_t;

			cairo_surface_t* cairo_image_surface_create(cairo_format_t, int, int);
			void cairo_surface_destroy(cairo_surface_t*);
			cairo_t* cairo_create(cairo_surface_t*);
			void cairo_destroy(cairo_t*);
			PangoLayout* pango_cairo_create_layout(cairo_t*);
			void g_object_unref(gpointer);
			PangoFontDescription* pango_font_description_new(void);
			void pango_font_description_free(PangoFontDescription*);
			void pango_font_description_set_family(PangoFontDescription*, const char*);
			void pango_font_description_set_weight(PangoFontDescription*, PangoWeight);
			void pango_font_description_set_style(PangoFontDescription*, PangoStyle);
			void pango_font_description_set_absolute_size(PangoFontDescription*, double);
			void pango_layout_set_font_description(PangoLayout*, PangoFontDescription*);
			PangoAttrList* pango_attr_list_new(void);
			void pango_attr_list_unref(PangoAttrList*);
			void pango_attr_list_insert(PangoAttrList*, PangoAttribute*);
			PangoAttribute* pango_attr_underline_new(PangoUnderline);
			PangoAttribute* pango_attr_strikethrough_new(gboolean);
			PangoAttribute* pango_attr_letter_spacing_new(int);
			void pango_layout_set_attributes(PangoLayout*, PangoAttrList*);
			PangoContext* pango_layout_get_context(PangoLayout*);
			const PangoFontDescription* pango_layout_get_font_description(PangoLayout*);
			PangoFontMetrics* pango_context_get_metrics(PangoContext*, const PangoFontDescription*, PangoLanguage*);
			void pango_font_metrics_unref(PangoFontMetrics*);
			int pango_font_metrics_get_ascent(PangoFontMetrics*);
			int pango_font_metrics_get_descent(PangoFontMetrics*);
			int pango_layout_get_spacing(PangoLayout*);
			void pango_layout_set_text(PangoLayout*, const char*, int);
			void pango_layout_get_pixel_extents(PangoLayout*, PangoRectangle*, PangoRectangle*);
			void cairo_save(cairo_t*);
			void cairo_restore(cairo_t*);
			void cairo_scale(cairo_t*, double, double);
			void pango_cairo_layout_path(cairo_t*, PangoLayout*);
			void cairo_new_path(cairo_t*);
			cairo_path_t* cairo_copy_path(cairo_t*);
			void cairo_path_destroy(cairo_path_t*);
			]] )
		end )
		pcall( function( )
			fontconfig = ffix.load( "fontconfig" )
			ffix.cdef( [[
			typedef void FcConfig;
			typedef void FcPattern;
			typedef struct{
				int nobject;
				int sobject;
				const char** objects;
			}FcObjectSet;
			typedef struct{
				int nfont;
				int sfont;
				FcPattern** fonts;
			}FcFontSet;
			typedef enum{
				FcResultMatch,
				FcResultNoMatch,
				FcResultTypeMismatch,
				FcResultNoId,
				FcResultOutOfMemory
			}FcResult;
			typedef unsigned char FcChar8;
			typedef int FcBool;

			FcConfig* FcInitLoadConfigAndFonts(void);
			FcPattern* FcPatternCreate(void);
			void FcPatternDestroy(FcPattern*);
			FcObjectSet* FcObjectSetBuild(const char*, ...);
			void FcObjectSetDestroy(FcObjectSet*);
			FcFontSet* FcFontList(FcConfig*, FcPattern*, FcObjectSet*);
			void FcFontSetDestroy(FcFontSet*);
			FcResult FcPatternGetString(FcPattern*, const char*, int, FcChar8**);
			FcResult FcPatternGetBool(FcPattern*, const char*, int, FcBool*);
			]] )
		end )
	end

	local libpng
	pcall( function( )
		libpng = ffix.load( LIBPNG_PATH )
		ffix.cdef( [[
		static const int PNG_SIGNATURE_SIZE = 8;
		typedef unsigned char png_byte;
		typedef png_byte* png_bytep;
		typedef const png_bytep png_const_bytep;
		typedef unsigned int png_size_t;
		typedef char png_char;
		typedef png_char* png_charp;
		typedef const png_charp png_const_charp;
		typedef void png_void;
		typedef png_void* png_voidp;
		typedef struct png_struct* png_structp;
		typedef const png_structp png_const_structp;
		typedef struct png_info* png_infop;
		typedef const png_infop png_const_infop;
		typedef unsigned int png_uint_32;
		typedef void (__cdecl *png_error_ptr)(png_structp, png_const_charp);
		typedef void (__cdecl *png_rw_ptr)(png_structp, png_bytep, png_size_t);
		enum{
			PNG_TRANSFORM_STRIP_16 = 0x1,
			PNG_TRANSFORM_PACKING = 0x4,
			PNG_TRANSFORM_EXPAND = 0x10,
			PNG_TRANSFORM_BGR = 0x80
		};
		enum{
			PNG_COLOR_MASK_COLOR = 2,
			PNG_COLOR_MASK_ALPHA = 4
		};
		enum{
			PNG_COLOR_TYPE_RGB = PNG_COLOR_MASK_COLOR,
			PNG_COLOR_TYPE_RGBA = PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA
		};

		void* memcpy(void*, const void*, size_t);
		int png_sig_cmp(png_const_bytep, png_size_t, png_size_t);
		png_structp png_create_read_struct(png_const_charp, png_voidp, png_error_ptr, png_error_ptr);
		void png_destroy_read_struct(png_structp*, png_infop*, png_infop*);
		png_infop png_create_info_struct(png_structp);
		void png_set_read_fn(png_structp, png_voidp, png_rw_ptr);
		void png_read_png(png_structp, png_infop, int, png_voidp);
		int png_set_interlace_handling(png_structp);
		void png_read_update_info(png_structp, png_infop);
		png_uint_32 png_get_image_width(png_const_structp, png_const_infop);
		png_uint_32 png_get_image_height(png_const_structp, png_const_infop);
		png_byte png_get_color_type(png_const_structp, png_const_infop);
		png_size_t png_get_rowbytes(png_const_structp, png_const_infop);
		png_bytep* png_get_rows(png_const_structp, png_const_infop);
		]] )
	end )
	
	-- Helper and short functions
	local pi = math.pi						-- el valor de la constante pi
	local ln = math.log						-- logaritmo (base e por default)
	local sin = math.sin					-- seno de un ángulo medido en radianes
	local cos = math.cos					-- coseno de un ángulo medido en radianes
	local tan = math.tan					-- tangente de un ángulo medido en radianes
	local abs = math.abs					-- valor absoluto de un número
	local deg = math.deg					-- convierte un ángulo de radianes a sexagesimal
	local rad = math.rad					-- convierte un ángulo de sexagesimal a radianes
	local log = math.log10					-- logaritmo en base 10
	local asin = math.asin					-- arcoseno
	local acos = math.acos					-- arcocoseno
	local atan = math.atan					-- arcotangente
	local sinh = math.sinh					-- seno hiperbólico
	local cosh = math.cosh					-- coseno hiperbólico
	local tanh = math.tanh					-- tangente hiperbólica
	local rand = math.random				-- número aleatorio
	local ceil = math.ceil					-- redondea al entero mayor más cercano
	local floor = math.floor				-- redondea al entero menor más cercano
	local atan2 = math.atan2				-- arcotangente a partir de dos valores
	local format = string.format			-- asigna formato a un string
	local unpack = table.unpack or unpack	-- extrae los elementos de una tabla indexada
	
	temp, recall = { }, { }
	local function set_temp( ref, val )
		temp[ ref ] = val
		return val
	end
	
	local function remember( ref, val )
		recall[ ref ] = val
		return val
	end

	local ratio, frame_dur = 1, 41.708
	
	local function rotate2d( x, y, angle )
		local ra = rad( angle )
		return cos( ra ) * x - sin( ra ) * y, sin( ra ) * x + cos( ra ) * y
	end
	
	local function bton( s )
		local bytes, n = { s:byte( 1, -1 ) }, 0
		for i = 0, #s - 1 do
			n = n + bytes[ 1 + i ] * 256 ^ i
		end
		return n
	end
	
	local function utf8_to_utf16( s )
		local wlen = ffix.C.MultiByteToWideChar( ffix.C.CP_UTF8X, 0x0, s, -1, nil, 0 )
		local ws = ffix.new( "wchar_t[?]", wlen )
		ffix.C.MultiByteToWideChar( ffix.C.CP_UTF8X, 0x0, s, -1, ws, wlen )
		return ws
	end
	
	local function utf16_to_utf8( ws )
		local slen = ffix.C.WideCharToMultiByte( ffix.C.CP_UTF8X, 0x0, ws, -1, nil, 0, nil, nil )
		local s = ffix.new( "char[?]", slen )
		ffix.C.WideCharToMultiByte( ffix.C.CP_UTF8X, 0x0, ws, -1, s, slen, nil, nil )
		return ffix.string( s )
	end

	-- Create library
	local ke4
	ke4 = {
		-- Time library
		time = {
			HMS_to_ms = function( time_HMS )
				--convierte el tiempo de HMS a ms
				if type( time_HMS ) == "string" then
					if time_HMS:match( "%d+%:%d+%:%d+%.%d+" ) then
						local H, M, S, ms = time_HMS:match( "(%d+)%:(%d+)%:(%d+)%.(%d+)" )
						if ms:len( ) == 2 then
							ms = 10 * ms
						elseif ms:len( ) == 1 then
							ms = 100 * ms
						end
						return H * 3600000 + M * 60000 + S * 1000 + ms
					end
					return tonumber( time_HMS )
				elseif type( time_HMS ) == "table" then
					if type( time_HMS[ 1 ] ) == "string" then
						if time_HMS[ 1 ]:match( "%d+%:%d+%:%d+%.%d+" ) then
							local H, M, S, ms = time_HMS[ 1 ]:match( "(%d+)%:(%d+)%:(%d+)%.(%d+)" )
							if ms:len( ) == 2 then
								ms = 10 * ms
							elseif ms:len( ) == 1 then
								ms = 100 * ms
							end
							time_HMS[ 1 ] = H * 3600000 + M * 60000 + S * 1000 + ms
						else
							time_HMS[ 1 ] = tonumber( time_HMS[ 1 ] )
						end
					end
					if type( time_HMS[ 2 ] ) == "string" then
						if time_HMS[ 2 ]:match( "%d+%:%d+%:%d+%.%d+" ) then
							local H, M, S, ms = time_HMS[ 2 ]:match( "(%d+)%:(%d+)%:(%d+)%.(%d+)" )
							if ms:len( ) == 2 then
								ms = 10 * ms
							elseif ms:len( ) == 1 then
								ms = 100 * ms
							end
							time_HMS[ 2 ] = H * 3600000 + M * 60000 + S * 1000 + ms - time_HMS[ 1 ]
						else
							time_HMS[ 2 ] = tonumber( time_HMS[ 2 ] )
						end
					end
					return { time_HMS[ 1 ], time_HMS[ 2 ] }
				end
				return time_HMS
			end, --!_G.ke4.time.HMS_to_ms( "0:03:37.134" )!
			
			ms_to_HMS = function( time_ms )
				--convierte el tiempo de ms a formato HMS
				local time_to_HMS
				if type( time_ms ) == "table" then
					local rec_table = { }
					for i = 1, #time_ms do
						rec_table[ i ] = ke4.time.ms_to_HMS( time_ms[ i ] )
					end
					time_to_HMS = rec_table
				else
					local tms, time_H, time_M, time_S = ke4.math.round( tonumber( time_ms ) ), 0, 0, 0
					time_H = floor( tms / 3600000 )
					tms = tms - time_H * 3600000
					time_M = floor( tms / 60000 )
					tms = tms - time_M * 60000
					time_S = floor( tms / 1000 )
					tms = tms - time_S * 1000
					if time_M < 10 then
						time_M = "0" .. time_M
					end
					if time_S < 10 then
						time_S = "0" .. time_S
					end
					if tostring( tms ):len( ) == 1 then
						tms = "00" .. tms
					elseif tostring( tms ):len( ) == 2 then
						tms =  "0" .. tms
					end
					time_to_HMS = format( "%s:%s:%s.%s", time_H, time_M, time_S, tms )
				end --!_G.ke4.time.ms_to_HMS( { 78656, 128716 } )!
				return time_to_HMS
			end, --!_G.ke4.time.ms_to_HMS( 128716 )!
			
			time_to_frame = function( Time )
				--retorna la cantidad de frames que hay en un tiempo determinado
				local t_to_frame
				if type( Time ) == "table" then
					local rec_table = { }
					for i = 1, #Time do
						rec_table[ i ] = ke4.time.time_to_frame( Time[ i ] )
					end
					t_to_frame = rec_table
				else
					-- frame_dur ---------------------------
					local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
					if msb then
						frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
					end
					----------------------------------------
					local Time = tostring( Time )
					if Time:match( "%d+%:%d+%:%d+%.%d+" ) then
						if type( ke4.time.HMS_to_ms( Time ) ) == "table" then
							Time = ke4.time.HMS_to_ms( Time )[ 1 ]
						else
							Time = ke4.time.HMS_to_ms( Time )
						end
					end
					t_to_frame = ceil( Time / frame_dur )
				end
				return t_to_frame
			end, --!_G.ke4.time.time_to_frame( 128716 )!
			
			frame_to_ms = function( frames )
				--convierte la cantidad de frames en un tiempo en formato ms
				local f_to_ms
				if type( frames ) == "table" then
					local rec_table = { }
					for i = 1, #frames do
						rec_table[ i ] = ke4.time.frame_to_ms( frames[ i ] )
					end
					f_to_ms = rec_table
				else --recursividad: september 10th 2019
					-- frame_dur ---------------------------
					local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
					if msb then
						frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
					end
					----------------------------------------
					f_to_ms =  ke4.math.round( frames * frame_dur, 2 )
				end --!_G.ke4.time.frame_to_ms( { 2365, 128, 82351 } )!
				return f_to_ms
			end, --!_G.ke4.time.frame_to_ms( 871 )!
			
			frame_to_HMS = function( frames )
				--convierte la cantidad de frames en un tiempo en formato HMS
				local f_to_HMS
				if type( frames ) == "table" then
					local rec_table = { }
					for i = 1, #frames do
						rec_table[ i ] = ke4.time.frame_to_HMS( frames[ i ] )
					end
					f_to_HMS = rec_table
				else
					f_to_HMS = ke4.time.ms_to_HMS( ke4.time.frame_to_ms( frames ) )
				end --!_G.ke4.time.frame_to_HMS( { 35, 240, { 4532, { 24, 276 }, 9574 } } )!
				return f_to_HMS
			end, --!_G.ke4.time.frame_to_HMS( 871 )!
			
			mid1 = function( val_i, val_n, Delay )
				-- extremos --> centro
				if type( Delay ) == "function" then
					Delay = Delay( )
				end
				local Delay = Delay or 60
				if val_i <= (val_n + 1) / 2 then
					return Delay * (val_i - 1) - 200
				end
				return Delay * (val_n - val_i + 0.5) - 200
			end, --!retime( "start2syl", _G.ke4.time.mid1( $si, $syln, 60 ), 0 )!

			mid2 = function( val_i, val_n, Delay )
				-- centro --> extremos
				if type( Delay ) == "function" then
					Delay = Delay( )
				end
				local Delay = Delay or 60
				if val_i >= (val_n + 1) / 2 then
					return Delay * (val_i - val_n - 1) - 200
				end
				return Delay * (-val_i + 0.5) - 200
			end, --!retime( "start2syl", _G.ke4.time.mid2( $si, $syln, 60 ), 0 )!
			
			li = function( val_i, val_n, Delay )
				if type( Delay ) == "function" then
					Delay = Delay( )
				end
				local Delay = Delay or 40
				return Delay * (val_i - 1) - 200
			end, --!_G.ke4.time.li( $si, $syln, 50 )!
			
			lo = function( val_i, val_n, Delay )
				if type( Delay ) == "function" then
					Delay = Delay( )
				end
				local Delay = Delay or 40
				return Delay * (val_i - val_n - 1) + 200
			end, --!_G.ke4.time.lo( $si, $syln, 50 )!
			
			loop = function( j, maxj, Delay, Mode )
				if type( Delay ) == "function" then
					Delay = Delay( )
				end
				local Mode = Mode or "li"
				local Delay = Delay or 30
				if Mode == "li" then
					return Delay * (j - 1) - 200
				elseif Mode == "lo" then
					return Delay * (j - maxj - 1) + 200
				elseif Mode == "mid1" then
					if j <= (maxj + 1) / 2 then
						return Delay * (j - 1) - 200
					end
					return Delay * (maxj - j + 0.5) - 200
				elseif Mode == "mid2" then
					if j >= (maxj + 1) / 2 then
						return Delay * (j - maxj - 1) - 200
					end
					return Delay * (-j + 0.5) - 200
				end
				return Delay
			end,
			
		},
		
		-- Table sublibrary
		table = {
			ipol = function( Table, Size, Tags, algorithm )
				--retorna una tabla con la interpolación de los valores de la tabla ingresada
				--interpola números, shapes, clips vectoriales, clips rectangulares, colores y transparencias
				local Table = Table or { 0, 1 }
				local Size = ceil( abs( Size or 10 ) )
				if #Table == 1 then
					Table[ 2 ] = Table[ 1 ]
				end
				if Size < #Table then
					Size = #Table
				end
				local algorithm = algorithm or "%s"
				-- algorithm example: "%s ^ 0.5"
				local function tbl_ipol_funct( Table_ipol, Size_ipol, Tags_ipol, algorithm_ipol, Shp_or_Clip )
					---------------------------------------------
					local ipols = { }
					local max_loop = Size_ipol - 1
					local ipol_i, ipol_f, pct_ip
					---------------------------------------------
					-- interpola el valor de dos números
					local function ipol_number( val_1, val_2, pct_ipol )
						return val_1 + (val_2 - val_1) * pct_ipol
					end
					---------------------------------------------
					-- interpola el valor de dos shapes o dos clips
					local function ipol_shpclip( val_1, val_2, pct_ipol )
						local val_1, val_2 = ke4.shape.insert( val_1, val_2 )
						local tbl_1, tbl_2, k = { }, { }, 1
						for c in val_1:gmatch( "%-?%d+[%.%d]*" ) do
							table.insert( tbl_1, tonumber( c ) )
						end
						for c in val_2:gmatch( "%-?%d+[%.%d]*" ) do
							table.insert( tbl_2, tonumber( c ) )
						end
						local val_ipol = val_1:gsub( "%-?%d+[%.%d]*",
							function( val )
								local val = tbl_1[ k ] + (tbl_2[ k ] - tbl_1[ k ]) * pct_ipol
								k = k + 1
								return ke4.math.round( val, 3 )
							end
						)
						return val_ipol
					end
					---------------------------------------------
					-- busca un string dentro de la tabla
					local function string_in_tbl( str_in_tbl )
						for i = 1, #str_in_tbl do
							if type( str_in_tbl[ i ] ) == "string" then
								return true, str_in_tbl[ i ]
							end
						end
						return false
					end
					---------------------------------------------
					-- decide cuál de las 4 interpolaciones se usará
					local ipol_function = ipol_number
					if Shp_or_Clip then
						ipol_function = ipol_shpclip
					end
					local ipol_coloralpha = ""
					if string_in_tbl( Table_ipol ) then
						local True, Val_str = string_in_tbl( Table_ipol )
						if Val_str:match( "[&Hh%#]^*%x%x%x%x%x%x[&]*" ) then
							ipol_function = ke4.color.interpolate
							ipol_coloralpha = "coloralpha"
						elseif Val_str:match( "[&Hh%#]^*%x%x[&]*" ) then
							ipol_function = ke4.alpha.interpolate
							ipol_coloralpha = "coloralpha"
						end
					end
					---------------------------------------------
					for i = 1, max_loop do
						--algoritmos de interpolación :D
						ipol_i = Table_ipol[ floor( (i - 1) / (max_loop / (#Table_ipol - 1)) ) + 1 ]
						ipol_f = Table_ipol[ floor( (i - 1) / (max_loop / (#Table_ipol - 1)) ) + 2 ]
						pct_ip = floor( (i - 1) % (max_loop / (#Table_ipol - 1)) ) / (max_loop / (#Table_ipol - 1))
						--ipols[ i ] = ke4.math.round( ipol_function( ipol_i, ipol_f, ke4.math.format( algorithm_ipol, pct_ip ) ), 3 )
						ipols[ i ] = ipol_function( ipol_i, ipol_f, ke4.math.format( algorithm_ipol, pct_ip ) )
					end --!_G.ke4.table.view( _G.ke4.table.ipol( { 12, 31 }, 11, "\\fscy", "math.sin( math.pi * %s )" ) )!
					pct_ip = ke4.math.clamp( ke4.math.format( algorithm_ipol, 1 ) ) * (#Table_ipol - 1)
					ipol_i = Table_ipol[ floor( pct_ip ) ]
					ipol_f = Table_ipol[ floor( pct_ip ) + 1 ] or Table_ipol[ floor( pct_ip ) ]
					ipols[ #ipols + 1 ] = ipol_function( ipol_i, ipol_f, ke4.math.clamp( ke4.math.format( algorithm_ipol, 1 ) ) )
					--concatena los valores con los Tags_ipol, si los hay
					if Tags_ipol then
						return ke4.table.concat2( ipols, Tags_ipol )
					end
					---------------------------------------------
					return ipols
					--!_G.ke4.table.view( _G.ke4.table.ipol( { 12, 31, 20, 13, 47 }, 16, "\\fscy" ) )!
				end
				-------------------------------------------------------------
				-- determina si los elementos son tablas, clip's o shapes
				local function type_table( Table )
					if type( Table[ 1 ] ) == "table" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "table" then
								return "mixed"
							end
						end
						return "table"
					elseif type( Table[ 1 ] ) == "string" then
						if Table[ 1 ]:match( "%([ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%)" ) then
							for i = 1, #Table do
								if not Table[ i ]:match( "%([ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%)" ) then
									return "mixed"
								end
							end
							return "clip"
						elseif Table[ 1 ]:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
							for i = 1, #Table do
								if not Table[ i ]:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
									return "mixed"
								end
							end
							return "shape"
						end
					end --type_table( { {1,2}, {3,4}, {5,6} } )
					return "others"
				end
				-------------------------------------------------------------
				if type_table( Table ) == "table" then
					local tbls_ipol, Tags_tbl = { }, Tags
					for i = 1, #Table do
						if type( Tags ) == "table" then
							Tags_tbl = Tags[ i ] or ""
						end
						if type_table( Table[ i ] ) == "shape"
							or type_table( Table[ i ] ) == "clip" then
							tbls_ipol[ i ] = tbl_ipol_funct( Table[ i ], Size, Tags_tbl, algorithm, true )
						else
							tbls_ipol[ i ] = tbl_ipol_funct( Table[ i ], Size, Tags_tbl, algorithm )
						end
					end
					return ke4.table.concat4( tbls_ipol )
				elseif type_table( Table ) == "shape"
					or type_table( Table ) == "clip" then
					return tbl_ipol_funct( Table, Size, Tags, algorithm, true )
				end
				return tbl_ipol_funct( Table, Size, Tags, algorithm )
			end,

			ipol2 = function( Table, Size, Tags, algorithm )
				--retorna una tabla con la interpolación de los valores de la tabla ingresada
				--interpola números, shapes, clips vectoriales, clips rectangulares, colores y transparencias
				local Table = Table or { 0, 1 }
				local Size = ceil( abs( Size or 10 ) )
				if #Table == 1 then
					Table[ 2 ] = Table[ 1 ]
				end
				if Size < #Table then
					Size = #Table
				end
				local algorithm = algorithm or "%s" -- algorithm example: "%s ^ 0.5"
				local function tbl_ipol_funct( Table_ipol, Size_ipol, Tags_ipol, algorithm_ipol )
					local ipols = { }
					for i = 1, Size_ipol do
						ipols[ i ] = ke4.math.round( ke4.tag.ipol( ke4.math.format( algorithm_ipol, (i - 1) / (Size_ipol - 1) ), Table_ipol ), 3 )
					end
					if Tags_ipol then --concatena los valores con los Tags_ipol, si los hay
						return ke4.table.concat2( ipols, Tags_ipol )
					end
					return ipols
				end
				-- determina si los elementos son tablas, clip's o shapes
				local function type_table( Table )
					if type( Table[ 1 ] ) == "table" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "table" then
								return "mixed"
							end
						end
						return "table"
					end
					return "others"
				end
				-------------------------------------------------------------
				if type_table( Table ) == "table" then
					local tbls_ipol, Tags_tbl = { }, Tags
					for i = 1, #Table do
						if type( Tags ) == "table" then
							Tags_tbl = Tags[ i ] or ""
						end
						itbls_ipol[ i ] = tbl_ipol_funct( Table[ i ], Size, Tags_tbl, algorithm )
					end
					return ke4.table.concat4( tbls_ipol )
				end
				return tbl_ipol_funct( Table, Size, Tags, algorithm )
			end, --!_G.ke4.table.view( _G.ke4.table.ipol2( { 12, 31, 20, 13, 47 }, 21, "\\fscy" ) )!

			make = function( objet, size, limit_i, limit_f, ... )
				--crea una tabla de diferentes objetos por valores predeterminados ordenados
				local t_make, Tme_concat, Tm_n = { }, { [ 1 ] = "" }, 0.5
				local i_c, i_a, _c, _a = ke4.color.ipolfx, ke4.alpha.ipolfx, ke4.color.vc_to_c, ke4.alpha.va_to_a
				if ... then
					Tme_concat = { ... }
					if type( ... ) == "table" then
						Tme_concat = ...
					end
				end
				if type( limit_i ) ~= "table" then 
					for i = 1, size do
						Tm_n = (i - 1) / (size - 1)
						if size == 1 then
							Tm_n = 0.5
						end
						if objet == "number" then
							t_make[ i ] = ke4.math.round( size * Tm_n, 2 )
							if limit_i then
								t_make[ i ] = ke4.math.round( limit_i + Tm_n * (limit_f - limit_i), 2 )
							end
						elseif objet == "color"
							or objet == "colorc"
							or objet == "colorvc" then
							t_make[ i ] = ke4.random.color( 360 * Tm_n )
							if type( limit_i ) == "string" then
								t_make[ i ] = i_c( Tm_n, _c( limit_i ), _c( limit_f ) )
							elseif type( limit_i ) == "number" then
								t_make[ i ] = ke4.random.color( limit_i + Tm_n * (limit_f - limit_i) )
							end
						elseif objet == "alpha"
							or objet == "Alphaa"
							or objet == "alphava" then
							t_make[ i ] = ke4.alpha.val2ass( 255 * Tm_n )
							if type( limit_i ) == "string" then
								t_make[ i ] = i_a( Tm_n, _a( limit_i ), _a( limit_f ))
							elseif type( limit_i ) == "number" then
								t_make[ i ] = ke4.alpha.val2ass( limit_i + Tm_n * (limit_f - limit_i) )
							end
						else
							t_make[ i ] = objet .. size
							if limit_i then
								t_make[ i ] = objet .. ke4.math.round( limit_i + Tm_n * (limit_f - limit_i), 2 )
							end
						end
					end
				else
					Tme_concat = limit_f
					t_make = ke4.table.gradient( size, limit_i )
				end
				t_make = ke4.table.concat2( t_make, Tme_concat or "" )
				return t_make
			end,
			
			rmake = function( objet, size, limit_i, limit_f, ... )
				--crea una tabla de diferentes objetos por valores predeterminados aleatoreamente
				local t_rmake, Trme_concat = { }, { [ 1 ] = "" }
				local i_c, i_a, _c, _a = ke4.color.ipolfx, ke4.alpha.ipolfx, ke4.color.vc_to_c, ke4.alpha.va_to_a
				if ... then
					Trme_concat = { ... }
					if type( ... ) == "table" then
						Trme_concat = ...
					end
				end
				if type( limit_i ) ~= "table" then 
					for i = 1, size do
						t_rmake[ i ] = ""
						for k = 1, #Trme_concat do
							if objet == "number" then
								t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. ke4.math.R( size )
								if limit_i then
									t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. ke4.math.Rc( limit_i, limit_f )
								end
							elseif objet == "color"
								or objet == "colorc"
								or objet == "colorvc" then
								t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. ke4.random.color( )
								if type( limit_i ) == "string" then
									t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. i_c( ke4.math.R( ), _c( limit_i ), _c( limit_f ) )
								elseif type( limit_i ) == "number" then
									t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. ke4.random.color( { limit_i, limit_f } )
								end
							elseif objet == "alpha"
								or objet == "Alphaa"
								or objet == "alphava" then
								t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. ke4.random.alpha( )
								if type( limit_i ) == "string" then
									t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. i_a( ke4.math.R( ), _a( limit_i ), _a( limit_f ) )
								elseif type( limit_i ) == "number" then
									t_rmake[ i ] = t_rmake[ i ] .. Trme_concat[ k ] .. ke4.random.alpha( limit_i, limit_f )
								end
							else
								t_rmake[ i ] = objet .. ke4.random.Rc( size )
								if limit_i then
									t_rmake[ i ] = objet .. ke4.math.round( ke4.math.R( limit_i, limit_f ), 2 )
								end
							end
						end
					end
				else
					Trme_concat = limit_f
					t_rmake = ke4.table.concat2( ke4.table.disorder( ke4.table.gradient( size, limit_i ) ), Trme_concat or "" )
				end
				return t_rmake
			end,
			
			duplicate = function( Table )
				--duplica completamente el contenido de una tabla
				local lookup_table = { }
				local function table_copy( Table )
					if type( Table ) ~= "table" then
						return Table
					elseif lookup_table[ Table ] then
						return lookup_table[ Table ]
					end
					local new_table = { }
					lookup_table[ Table ] = new_table
					for k, v in pairs( Table ) do
						new_table[ table_copy( k ) ] = table_copy( v )
					end
					return setmetatable( new_table, getmetatable( Table ) )
				end
				return table_copy( Table )
			end,

			op = function( Table, mode, add )
				--realiza múltiples operaciones con los elementos de la tabla ingresada
				if type( Table ) == "function" then
					Table = Table( )
				end
				local table_sum, table_average, table_concat = 0, 0, ""
				local table_add, table_inverse, table_function = { }, { }, { }
				if type( mode ) == "function" then
					--retorna una tabla en donde a cada elemento se le aplica la función mode( val )
					for k, v in pairs( Table ) do
						table_function[ k ] = mode( v )
					end
					return table_function --may 23rd 2018
				elseif mode == "sum"
					or mode == "suma"
					or mode == nil then
					--retorna el valor de la suma de los elemente desde 1 hasta add o #Table
					-------------------
					local add = add or #Table
					if add > #Table then
						add = #Table
					end --september 30th 2018
					-------------------
					for i = 1, add do
						table_sum = table_sum + Table[ i ]
					end
					return table_sum
				elseif mode == "pro"
					or mode == "multi" then
					--retorna una tabla con cada elemento multiplicado por un mismo número
					local table_pro = ke4.table.duplicate( Table )
					for i = 1, #Table do
						table_pro[ i ] = Table[ i ] * add
					end
					return table_pro
				elseif mode == "concat" then
					--retorna un string equivalente a todos los elementos de la tabla concatenados
					local con_add = ""
					for i = 1, #Table do
						con_add = ""
						if add
							and i < #Table then
							con_add = add
						end
						table_concat = table_concat .. Table[ i ] .. con_add
					end
					return table_concat
				elseif mode == "average" then
					--retorna un número equivalente al promedio aritmético de los números de la tabla
					for i = 1, #Table do
						table_average = table_average + Table[ i ]
					end
					if #Table > 0 then
						return table_average / #Table
					end
					return 0
				elseif mode == "min" then
					--retorna el mínimo valor de la tabla ingresada
					local table_min = ke4.table.duplicate( Table )
					table.sort( table_min, function( a, b ) return a < b end )
					if table_min[ 1 ] then
						return table_min[ 1 ]
					end
					return 0
				elseif mode == "max" then
					--retorna el máximo valor de la tabla ingresada
					local table_max = ke4.table.duplicate( Table )
					table.sort( table_max, function( a, b ) return a < b end )
					if table_max[ #table_max ] then
						return table_max[ #table_max ]
					end
					return 0
				elseif mode == "rank" then
					--retorna el rango de los valores de la tabla ingresada
					local table_rank = ke4.table.duplicate( Table )
					table.sort( table_rank, function( a, b ) return a < b end )
					if table_rank[ 1 ] then
						return table_rank[ #table_rank ] - table_rank[ 1 ]
					end
					return 0
				elseif mode == "org" then
					--retorna la tabla con los números organizados ascendentemente
					local table_org = ke4.table.duplicate( Table )
					table.sort( table_org, function( a, b ) return a < b end )
					return table_org
				elseif mode == "org2" then
					--retorna la tabla con los números organizados descendentemente
					local table_org2 = ke4.table.duplicate( Table )
					table.sort( table_org2, function( a, b ) return a > b end )
					return table_org2
				elseif mode == "round" then
					--retorna la tabla con todos los números redondeados según el Argumento 3
					local table_round = ke4.table.duplicate( Table )
					return ke4.math.round( table_round, add )
				elseif mode == "add" then
					--retorna la tabla, y a cada uno de los elementos le suma un número
					if type( add ) == "number" then
						for i = 1, #Table do
							table_add[ i ] = Table[ i ] + add
						end
					elseif type( add ) == "table" then
						for i = 1, #Table do 
							if type( Table[ i ] ) == "table" then
								table_add[ i ] = { }
								for k = 1, #Table[ i ] do
									table_add[ i ][ k ] = Table[ i ][ k ] + add[ 1 + (k + 1) % 2 ]
									if k % 2 == 1 then
										table_add[ i ][ k ] = Table[ i ][ k ] + add[ 1 + (k - 1) % 2 ]
									end
								end
							elseif i % 2 == 1 then
								table_add[ i ] = Table[ i ] + add[ 1 + (i - 1) % 2 ]
							else
								table_add[ i ] = Table[ i ] + add[ 1 + (i + 1) % 2 ]
							end
						end
					end
					return table_add
				elseif mode == "inverse" then
					--retorna la tabla con los elementos invertidos en el orden en que se ingresaron
					for i = 1, #Table do
						table_inverse[ i ] = Table[ #Table - i + 1 ]
					end
					return table_inverse
				end
			end,

			reverse = function( Table )
				--invierte el orden de los elementos de una tabla
				local Table_rev = { }
				for i = 1, #Table do
					Table_rev[ i ] = Table[ #Table - i + 1 ]
				end
				return Table_rev
			end,
			
			view = function( Table, Table_name, indent )
				--retorna en modo string el contenido completo de una tabla
				local cart
				local autoref
				local function isemptytable( Table )
					return next( Table ) == nil
				end
				local function basicSerialize( o )
					local so = tostring( o )
					if type( o ) == "function" then
						local info = debug.getinfo( o, "S" )
						if info.what == "C" then
							return format( "%q", so .. ", C function" )
						end 
						return format( "%q, defined in (lines: %s - %s), ubication %s",
							so, info.linedefined, info.lastlinedefined, info.source
						)
					elseif type( o ) == "number"
						or type( o ) == "boolean" then
						return so
					end
					return format( "%q", so )
				end
				local function addtocart( value, Table_name, indent, saved, field )
					indent = indent or ""
					saved = saved or { }
					field = field or Table_name
					cart = cart .. indent .. field
					if type( value ) ~= "table" then
						cart = cart .. " = " .. basicSerialize( value ) .. ";\n"
					else
						if saved[ value ] then
							cart = cart .. " = {}; -- " .. saved[ value ] .. " (self reference)\n"
							autoref = autoref ..  Table_name .. " = " .. saved[ value ] .. ";\n"
						else
							saved[ value ] = Table_name
							if isemptytable( value ) then
								cart = cart .. " = {};\n"
							else
								cart = cart .. " = {\n"
								for k, v in pairs( value ) do
									k = basicSerialize( k )
									local fname = format( "%s[ %s ]", Table_name, k )
									field = format( "[ %s ]", k )
									addtocart( v, fname, indent .. "	", saved, field )
								end
								cart = format( "%s%s};\n", cart, indent )
							end
						end
					end
				end
				Table_name = Table_name or "table_unnamed"
				if type( Table ) ~= "table" then
					return format( "%s = %s", Table_name, basicSerialize( Table ) )
				end
				cart, autoref = "", ""
				addtocart( Table, Table_name, indent )
				return cart .. autoref
			end,
			
			show = function( Table )		-- only for indexed table
				--retorna el contenido de una tabla, entre paréntesis y separados por comas (,)
				local t_show, t_show2 = "", ""
				for i = 1, #Table do
					if type( Table[ i ] ) == "table" then
						t_show2 = ""
						for k = 1, #Table[ i ] do
							t_show2 = format( "%s%s,", t_show2, Table[ i ][ k ] )
						end
						Table[ i ] = format( "(%s)", t_show2:sub( 1, -2 ) )
					end
					t_show = format( "%s%s,", t_show, Table[ i ] )
				end
				return t_show:sub( 1, -2 )
			end,

			compare = function( Table1, Table2 )
				--retorna "false" o "true" al comparar dos tablas
				if type( Table1 ) ~= type( Table2 ) then
					return false
				end
				if type( Table1 ) ~= "table"
					and type( Table2 ) ~= "table" then
					return Table1 == Table2
				end
				mt = getmetatable( Table1 )
				if mt
					and mt.__eq then
					return Table1 == Table2
				end
				for k1, val1 in pairs( Table1 ) do
					val2 = Table2[ k1 ]
					if val2 == nil
						or not ke4.table.compare( val1, val2 ) then
						return false
					end
				end
				for k2, val2 in pairs( Table2 ) do
					val1 = Table1[ k2 ]
					if val1 == nil
						or not ke4.table.compare( val1, val2 ) then
						return false
					end
				end
				return true
			end,

			inside = function( Table, e, str1, str2 )	-- only for indexed table
				--retorna "false" o "true" si un elemento está dentro una tabla
				local repl1 = str1 or ""
				local repl2 = str2 or ""
				for _, v in ipairs( Table ) do
					if type( e ) ~= "table" then
						if type( v ) == "string" then
							if e == v:gsub( repl1, repl2 ) then
								return true
							end
						else
							if e == v then
								return true
							end
						end
					else
						if ke4.table.compare( v, e ) then
							return true
						end
					end
				end
				return false
			end,
			
			index = function( Table, e, str1, str2 )	-- only for indexed table
				--retorna la posición (index) de un elemento dentro de una tabla
				local repl1 = str1 or ""
				local repl2 = str2 or ""
				if ke4.table.inside( Table, e, repl1, repl2 ) == true then
					for k, v in ipairs( Table ) do
						if type( e ) ~= "table" then
							if type( v ) == "string" then
								if e == v:gsub( repl1, repl2 ) then
									return k
								end
							else
								if e == v then
									return k
								end
							end
						else
							if ke4.table.compare( v, e ) then
								return k
							end
						end
					end
				end
				return e
			end,
			
			disorder = function( table_or_number )
				--desordena aleatoriamente el contenido de una tabla
				local Table_dis = table_or_number
				local newt, newtable, newt1, newtable1, table_n = { }, { }, { }, { }, { }
				if type( Table_dis ) == "table" then
					newt = ke4.table.duplicate( Table_dis )
					while #newt > 0 do
						idx = ke4.math.R( 1, #newt )
						newtable[ #newtable + 1 ] = newt[ idx ]
						table.remove( newt, idx )
					end
					return newtable
				elseif type( Table_dis ) == "number" then
					for i = 1, Table_dis do
						table_n[ i ] = i
					end
					newt1 = ke4.table.duplicate( table_n )
					while #newt1 > 0 do
						idx = ke4.math.R( 1, #newt1 )
						newtable1[ #newtable1 + 1 ] = newt1[ idx ]
						table.remove( newt1, idx )
					end
					return newtable1
				end
				return Table_dis
			end, --!_G.ke4.table.view( _G.ke4.table.disorder( { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 } ) )!
			
			complete = function( Table, Start_time, End_time )
				--ingresa el tiempo inicial y final de una línea kara
				--en una tabla de tiempos y los ordena ascendentemente
				local Table_com = ke4.table.duplicate( Table )
				for i = 1, #Table_com do
					Table_com[ i ] = ke4.time.HMS_to_ms( Table_com[ i ] )
				end
				table.insert( Table_com, Start_time )
				table.insert( Table_com, End_time )
				local B_com, nB, vB = ke4.table.duplicate( Table_com ), { }, { }
				for i = 1, #B_com do
					if type( B_com[ i ] ) == "table" then
						B_com[ i ] = B_com[ i ][ 1 ]
						table.insert( nB, i )
						table.insert( vB, B_com[ i ] )
					end
				end
				local C_com = ke4.table.duplicate( B_com )
				table.sort( C_com, function( a, b ) return a < b end )
				local D_com = ke4.table.duplicate( C_com )
				for i = 1, #D_com do
					if ke4.table.inside( vB, D_com[ i ] ) == true then
						D_com[ i ] = Table_com[ nB[ ke4.table.index( vB, D_com[ i ] ) ] ]
					end
				end
				return D_com
			end,
			
			filter = function( Table, Filter )
				--filtra los elementos de la tabla por medio de una función
				--retorna una tabla con los elementos filtrados de Table
				local tbl_filter = { }
				for k, v in pairs( Table ) do
					if Filter( k, v ) then
						table.insert( tbl_filter, v )
					end
				end
				return tbl_filter --table.filter( { 1, 2, 3, 4, 5 ,6 }, Filter ) <-- impares
				--Filter = function( k, v ) if k % 2 == 1 then return true end return false end
			end,

			replay = function( Len, ... )
				--> Len = 3; t = { a, b, c } --> return { a,b,c, a,b,c, a,b,c }
				--replica n cantidad de veces a los elementos de una tabla, o los ingresados
				local Len = ke4.math.round( abs( Len ) )
				local Table = { }
				local e_val = { ... }
				if type( ... ) == "table" then
					e_val = ...
				end
				for i = 1, Len do
					for k = 1, #e_val do
						if type( e_val[ k ] ) == "function" then --add: may 23rd 2018
							Table[ #Table + 1 ] = e_val[ k ]( )
						else
							Table[ #Table + 1 ] = e_val[ k ]
						end
					end
				end
				return Table
			end,

			concat1 = function( Table, ... )
				-->Table = {a, b, c}; ... = {1, 2, 3} -->return {1a,2a,3a, 1b,2b,3b, 1c,2c,3c}
				local Concat, Table_Concat = { [ 1 ] = "" }, { }
				if type( ... ) == "table" then
					Concat = ...
				else
					Concat = { ... }
				end
				for i = 1, #Table * #Concat do
					Table_Concat[ i ] = Concat[ #Concat - #Concat * ceil( i / #Concat ) + i ] .. Table[ ceil( i / #Concat ) ]
				end
				return Table_Concat
			end,
			
			concat2 = function( Table, ... )
				-->Table = {a, b, c}; ... = {1, 2, 3} -->return {1a2a3a, 1b2b3b, 1c2c3c}
				local Concat, Table_Concat, Table_Concat2 = { [ 1 ] = "" }, { }, { }
				if type( ... ) == "table" then
					Concat = ...
				else
					Concat = { ... }
				end
				for i = 1, #Table * #Concat do
					Table_Concat[ i ] = Concat[ #Concat - #Concat * ceil( i / #Concat ) + i ] .. Table[ ceil( i / #Concat ) ]
				end
				for i = 1, #Table do
					Table_Concat2[ i ] = ""
					for k = 1, #Concat do
						Table_Concat2[ i ] = Table_Concat2[ i ] .. Table_Concat[ k + #Concat * (i - 1) ]
					end
				end
				return Table_Concat2
			end,
			
			concat3 = function( ... )
				--"concatena" uno a uno los elementos de dos o más tablas
				local Tables = { ... }
				if #Tables == 1 then
					Tables = ...
				end
				local Sizes, max_n = { [ 1 ] = 1 }
				-- obtiene el tamaño de la tabla más grande
				for i = 1, #Tables do
					if type( Tables[ i ] ) == "table" then
						table.insert( Sizes, #Tables[ i ] )
					end
				end
				max_n = ke4.table.op( Sizes, "max" )
				-------------------------------------------
				-- convierte en tabla los elementos que no lo son
				for i = 1, #Tables do
					if type( Tables[ i ] ) ~= "table" then
						Tables[ i ] = ke4.table.replay( max_n, Tables[ i ] )
					end
				end
				-------------------------------------------
				-- concatena los elementos de cada tabla uno a uno
				local tbl_twin = { }
				for i = 1, max_n do
					tbl_twin[ i ] = ""
					for k = 1, #Tables do
						tbl_twin[ i ] = tbl_twin[ i ] .. Tables[ k ][ (i - 1) % #Tables[ k ] + 1 ]
					end
				end
				return tbl_twin --may 20th 2018
			end, --table.concat3( { "A", "B", "C", "D" }, "1", { "w", "x", "y", "z" } )
			
			concat4 = function( ... )
				-->table.concat4( { "a", "b", "c", "d" }, { 1, 2, 3 } ) = { a1, b2, c3, d }
				--concatena uno a uno los elementos de las tablas ingresadas
				--si no hay emparejamiento, concatena con un string vacío ( "" )
				local Table = { ... }
				if #Table == 1 then
					Table = ...
				end
				local tbl_sizes = { }
				for i = 1, #Table do
					tbl_sizes[ i ] = #Table[ i ]
				end
				local max_sizes = ke4.table.op( tbl_sizes, "max" )
				local tbl_concat4 = { }
				for i = 1, max_sizes do
					tbl_concat4[ i ] = ""
					for k = 1, #Table do
						tbl_concat4[ i ] = tbl_concat4[ i ] .. (Table[ k ][ i ] or "")
					end
				end
				return tbl_concat4
			end,
			
			count = function( Table, e )
				-- cuenta la cantidad de veces que está un elemento en una tabla
				local Count = 0
				for k, v in pairs( Table ) do
					if e == v then
						Count = Count + 1
					end
				end
				return Count
			end,
			
			pos = function( Table, e )
				--retorna una tabla con las posiciones del elemento "e"
				local Table_pos = { }
				for k, v in pairs( Table ) do
					if e == v then
						table.insert( Table_pos, k )
					end
				end
				return Table_pos
			end,
			
			string = function( String, Number_str )
				--> ( "String", 2 ) --> { St, tr, ri, in, ng }
				--retorna una tabla con las partes de n tamaño de un string
				local String = String or ""
				local Number = Number_str or 1
				local Table_str, chars_, Len_str = { }, { }, unicode.len( String )
				for c in unicode.chars( String ) do
					table.insert( chars_, c )
				end
				if Number >= Len_str then
					return { String }
				end
				for i = 1, Len_str - Number + 1 do
					Table_str[ i ] = ""
					for k = 1, Number do
						Table_str[ i ] = Table_str[ i ] .. chars_[ i + k - 1 ]
					end
				end
				return Table_str
			end,
			
			space = function( String )
				--retorna una tabla con las posciones de los espacios (" ") que contenga un string
				local Table_s, Table_spc = ke4.table.string( String ), { }
				for i = 1, #Table_s do
					if Table_s[ i ] == " " then
						table.insert( Table_spc, i )
					end
				end
				return Table_spc
			end,
			
			word = function( String )
				--retorna una tabla con las palabras de un string
				local Table_word = { }
				for word_s in String:gmatch( "%S+" ) do
					table.insert( Table_word, word_s )
				end
				return Table_word
			end,
			
			retire = function( Table, ... )
				--retira de una tabla los elementos indicados o los elementos
				--que estén desde la posición { { a, b } } consecutivos todos
				local Table_ret, retire_e = ke4.table.duplicate( Table ), { ... }
				if type( ... ) == "table" then
					retire_e = ...
				end
				if type( retire_e[ 1 ] ) == "table" then
					if retire_e[ 1 ][ 2 ] then
						for i = retire_e[ 1 ][ 1 ], retire_e[ 1 ][ 2 ] do
							table.remove( Table_ret, retire_e[ 1 ][ 1 ] )
						end
					else
						table.remove( Table_ret, retire_e[ 1 ][ 1 ] )
					end
				else
					for i = 1, #retire_e do
						while ke4.table.inside( Table_ret, retire_e[ i ] ) == true do
							table.remove( Table_ret, ke4.table.index( Table_ret, retire_e[ i ] ) )
						end
					end
				end
				return Table_ret
			end, --table.retire( { 21, 22, 23, 24, 25, 26 }, { { 1, 4 } } )
			
			combine = function( Table, n_combinations )
				--obtiene las combinaciones de n tamaño de una tabla
				local Table_com, a, nN = { }, { }, ke4.math.round( abs( n_combinations ) )
				local newrow
				for i = 1, nN do
					a[ #a + 1 ] = i
				end
				while ( 1 ) do
					newrow = { }
					for i = 1, nN do
						newrow[ #newrow + 1 ] = Table[ a[ i ] ]
					end
					Table_com[ #Table_com + 1 ] = newrow
					i = nN
					while a[ i ] == #Table - nN + i do
						i = i - 1
					end
					if i < 1 then
						break
					end
					a[ i ] = a[ i ] + 1
					for k = i, nN do
						a[ k ] = a[ i ] + k - i
					end
				end
				return Table_com --( {a,b,c,d}, 2 ) -> {{a,b}, {a,c}, {a,d}, {b,c}, {b,d}, {c,d}}
			end,
			
			inserttable = function( Table1, Table2, index_insert )
				--inserta los elementos de la tabla 2 dentro de la tabla 1
				local Table_ins = Table1
				if index_insert == nil then
					for i = 1, #Table2 do
						Table_ins[ #Table_ins + 1 ] = Table2[ i ]
					end
				else
					for i = 1, #Table2 do
						table.insert( Table_ins, index_insert, Table2[ #Table2 - i + 1 ] )
					end
				end
				return Table_ins
			end,

			reverse = function( Table )
				--invierte el orden de los elementos de una tabla
				local Table_rev = { }
				for i = 1, #Table do
					Table_rev[ i ] = Table[ #Table - i + 1 ]
				end
				return Table_rev
			end,
			
			cyclic = function( Table )
				--> {a,b,c,d,e} --> {a,b,c,d,e,d,c,b}
				--crea un "ciclo" con los elementos de una tabla
				if #Table <= 2 then
					return Table
				end
				local Table_cyc = ke4.table.duplicate( Table )
				for i = 1, #Table - 2 do
					Table_cyc[ #Table_cyc + 1 ] = Table[ #Table - i ]
				end
				return Table_cyc
			end,
			
			random = function( Table_or_Number )
				--retorna un elemento aleatoriamente de la tabla ingresada
				--o un número entero al azar entre 1 y el número ingresado
				local T_o_N = Table_or_Number or { 1 }
				if type( T_o_N ) == "number" then
					local Num = T_o_N
					T_o_N = { }
					for i = 1, Num do
						T_o_N[ i ] = i
					end
				end
				return T_o_N[ R( #T_o_N ) ]
			end, --_G.ke4.table.random( 9 )
			
			delete = function( Table, ... )
				--elimina el o los elementos, o una tabla de elementos
				--que estén dentro de una tabla seleccionada "Table"
				local tbl_delete, retire_e = ke4.table.duplicate( Table ), { ... }
				if type( ... ) == "table" then
					retire_e = ...
				end
				for i = 1, #retire_e do
					while ke4.table.inside( tbl_delete, retire_e[ i ] ) == true do
						table.remove( tbl_delete, ke4.table.index( tbl_delete, retire_e[ i ] ) )
					end
				end
				return tbl_delete
			end,

			permute = function( Table )
				--retorna una tabla que contiene las tablas con todas las
				--combinaciones posibles que se pueden hacer de la tabla ingresada
				--> { 1, 2, 3 } --> { {2, 3, 1}, {2, 1, 3}, {3, 1, 2}, {3, 2, 1}, {1, 3, 2}, {1, 2, 3} }
				local Table_Per = { }
				local function output( table_per )
					local tbl_inside = { }
					for _, val in ipairs( table_per ) do
						table.insert( tbl_inside, val )
					end
					table.insert( Table_Per, tbl_inside )
				end
				local function permutation( table_per, n )
					if n == 0 then
						output( table_per )
					else
						for i = 1, n do
							table_per[ n ], table_per[ i ] = table_per[ i ], table_per[ n ]
							permutation( table_per, n - 1 )
							table_per[ n ], table_per[ i ] = table_per[ i ], table_per[ n ]
						end
					end
				end
				local n = #Table
				permutation( Table, n )
				return Table_Per
			end,
			
			capture = function( String, Capture )
				--crea una tabla con las capturas de un String
				local tbl_cap = { }
				local String = String or ""
				local Capture = Capture or ""
				if type( Capture ) == "table" then
					for i = 1, #Capture do
						for cap in String:gmatch( Capture[ i ] ) do
							table.insert( tbl_cap, cap )
						end
					end
				else
					for cap in String:gmatch( Capture ) do
						table.insert( tbl_cap, cap )
					end
				end
				return tbl_cap
			end,

			gsub = function( Table, Capture, Replace )
				-- genera reemplazos en los elementos string de una tabla
				local Replace = Replace or ""
				local Capture = Capture or "KEfx"
				local tbl_gsub = { }
				local Table_gs = ke4.table.duplicate( Table )
				for k, val in pairs( Table_gs ) do
					if type( val ) == "string" then
						if type( Capture ) == "table" then
							for i = 1, #Capture do
								val = val:gsub( Capture[ i ], Replace )
							end
						else
							val = val:gsub( Capture, Replace )
						end
					end
					tbl_gsub[ k ] = val
				end
				return tbl_gsub
			end,

			match = function( Table, Capture )
				-- genera una tabla con las coincidencias que encuentre
				local tbl_match = { }
				local table_mch = ke4.table.duplicate( Table )
				if type( Capture ) == "table" then
					for i = 1, #Capture do
						if ke4.table.inside( table_mch, Capture[ i ] ) then
							table.insert( tbl_match, Capture[ i ] )
						else
							for _, val in pairs( table_mch ) do
								if Capture[ i ] == val then
									table.insert( tbl_match, Capture[ i ] )
								elseif type( val ) == "string"
									and type( Capture[ i ] ) == "string" then
									if val:match( Capture[ i ] ) then
										table.insert( tbl_match, val:match( Capture[ i ] ) )
									end
								end
							end
						end
					end
				else
					if ke4.table.inside( table_mch, Capture ) then
						table.insert( tbl_match, Capture )
					else
						for _, val in pairs( table_mch ) do
							if Capture == val then
								table.insert( tbl_match, Capture )
							elseif type( val ) == "string"
								and type( Capture ) == "string" then
								if val:match( Capture ) then
									table.insert( tbl_match, val:match( Capture ) )
								end
							end
						end
					end
				end
				return tbl_match
			end,
			
			unique = function( Table )
				--elimina los elementos repetidos de una tabla
				local tbl_uni = { }
				local tbl_cop = ke4.table.duplicate( Table )
				while #tbl_cop > 0 do
					table.insert( tbl_uni, tbl_cop[ 1 ] )
					tbl_cop = ke4.table.retire( tbl_cop, tbl_cop[ 1 ] )
				end
				return tbl_uni
			end, --_G.ke4.table.unique( { 1, 2, 2, 2, 5, 6, 7, 8 } )
			
			copy = function( t, depth )
				-- Copies table deep
				if type( t ) ~= "table" or depth ~= nil and not(type( depth ) == "number" and depth >= 1) then
					error( "table and optional depth expected", 2 )
				end
				local function copy_recursive( old_t )
					local new_t = { }
					for key, value in pairs( old_t ) do
						new_t[ key ] = type( value ) == "table" and copy_recursive( value ) or value
					end
					return new_t
				end
				local function copy_recursive_n( old_t, depth )
					local new_t = { }
					for key, value in pairs( old_t ) do
						new_t[ key ] = type( value ) == "table" and depth >= 2 and copy_recursive_n( value, depth - 1 ) or value
					end
					return new_t
				end
				return depth and copy_recursive_n( t, depth ) or copy_recursive( t )
			end,
			
			tostring = function( t )
				-- Converts table to string
				if type( t ) ~= "table" then
					error( "table expected", 2 )
				end
				local result, result_n = { }, 0
				local function convert_recursive( t, space )
					for key, value in pairs( t ) do
						if type( key ) == "string" then
							key = format( "%q", key )
						end
						if type( value ) == "string" then
							value = format( "%q", value )
						end
						result_n = result_n + 1
						result[ result_n ] = format( "%s[%s] = %s", space, key, value )
						if type( value ) == "table" then
							convert_recursive( value, space .. "\t" )
						end
					end
				end
				convert_recursive( t, "" )
				return table.concat( result, "\n" )
			end,
			
			gradient = function( Size, ... )
				-- example algorithm: "sin( pi * %s )"
				local toGradient = { ... }
				if type( ... ) == "table" then
					toGradient = ...
				end
				if #toGradient == 1 then
					toGradient[ 2 ] = "&HFFFFFF&"
				end
				toGradient = ke4.color.from_error( toGradient )
				local Size = Size or 12
				local Grad, Gradient_H, Gradient_V = { }, { }, { }
				local algorithm, Siz = "%s", ke4.math.round( Size )
				if type( Size ) == "table"
					and Size[ 2 ] then
					Siz = ke4.math.round( Size[ 1 ] )
					algorithm = Size[ 2 ]
				end
				if type( Size ) == "table"
					and type( Size[ 1 ] ) == "table" then
					Siz = ke4.math.round( Size[ 1 ][ 1 ] )
					for i = 1, Siz do
						Grad[ i ] = ke4.tag.ipol( ke4.math.format( algorithm, (i - 1) / (Siz - 1) ), toGradient )
					end
					return Grad
				end
				for i = 1, Siz do
					Grad[ i ] = ke4.tag.ipol( ke4.math.format( algorithm, (i - 1) / (Siz - 1) ), toGradient )
				end
				return Grad
			end, --\1c!_G.ke4.table.gradient( $syln, "&H0000FF&", "&HFFFFFF&", "&H00FFFF&" )[syl.i]!

			type = function( Table )
				if #Table > 0 then
					::go_to_ini::
					if type( Table[ 1 ] ) == "string" then
						--color
						if Table[ 1 ]:match( "%x%x%x%x%x%x" ) then
							local is_color = 0
							for i = 1, #Table do
								if type( Table[ i ] ) == "string"
									and Table[ i ]:match( "%x%x%x%x%x%x" ) then
									is_color = is_color + 1
								end
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "string" then
									if Table[ i ]( ):match( "%x%x%x%x%x%x" ) then
										is_color = is_color + 1
									end
								end
							end
							if is_color == #Table then
								return "color"
							end
						end
						--alpha
						if Table[ 1 ]:match( "%x%x" ) then
							local is_alpha = 0
							for i = 1, #Table do
								if type( Table[ i ] ) == "string"
									and Table[ i ]:match( "%x%x" ) then
									is_alpha = is_alpha + 1
								end
								if type( Table[ i ] ) == "function" then
									if type( Table[ i ]( ) ) == "string"
										and Table[ i ]( ):match( "%x%x" ) then
										is_alpha = is_alpha + 1
									elseif type( Table[ i ]( ) ) == "number" then
										is_alpha = is_alpha + 1
									end
								end
								if type( Table[ i ] ) == "number" then
									is_alpha = is_alpha + 1
								end
							end
							if is_alpha == #Table then
								return "alpha"
							end
						end
						--shape
						if Table[ 1 ]:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
							local is_shape = 0
							for i = 1, #Table do
								if type( Table[ i ] ) == "string"
									and Table[ i ]:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
									is_shape = is_shape + 1
								end
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "string" then
									if Table[ i ]( ):match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
										is_shape = is_shape + 1
									end
								end
							end
							if is_shape == #Table then
								return "shape"
							end
						end
						--clip
						if Table[ 1 ]:match( "%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*" ) then
							local is_clip = 0
							for i = 1, #Table do
								if type( Table[ i ] ) == "string"
									and Table[ i ]:match( "%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*" ) then
									is_clip = is_clip + 1
								end
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "string" then
									if Table[ i ]( ):match( "%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*" ) then
										is_clip = is_clip + 1
									end
								end
							end
							if is_clip == #Table then
								return "clip"
							end
						end
					end
					if type( Table[ 1 ] ) == "string" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "string" then
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "string" then
									goto is_string
								end
								return "mixed"
							end
							::is_string::
						end
						return "string"
					elseif type( Table[ 1 ] ) == "number" then
						--alpha
						local its_alpha = 0
						local min_alpha = 0
						for i = 1, #Table do
							if type( Table[ i ] ) == "string"
								and Table[ i ]:match( "%x%x" ) then
								its_alpha = its_alpha + 1
								min_alpha = min_alpha + 1
							end
							if type( Table[ i ] ) == "function" then
								if type( Table[ i ]( ) ) == "string"
									and Table[ i ]( ):match( "%x%x" ) then
									its_alpha = its_alpha + 1
									min_alpha = min_alpha + 1
								elseif type( Table[ i ]( ) ) == "number" then
									its_alpha = its_alpha + 1
								end
							end
							if type( Table[ i ] ) == "number" then
								its_alpha = its_alpha + 1
							end
						end
						if its_alpha == #Table
							and min_alpha > 0 then
							return "alpha"
						end
						--number
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "number" then
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "number" then
									goto is_number
								end
								return "mixed"
							end
							::is_number::
						end
						return "number"
					elseif type( Table[ 1 ] ) == "table" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "table" then
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "table" then
									goto is_table
								end
								return "mixed"
							end
							::is_table::
						end
						return "table"
					elseif type( Table[ 1 ] ) == "boolean" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "boolean" then
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "boolean" then
									goto is_boolean
								end
								return "mixed"
							end
							::is_boolean::
						end
						return "boolean"
					elseif type( Table[ 1 ] ) == "thread" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "thread" then
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "thread" then
									goto is_thread
								end
								return "mixed"
							end
							::is_thread::
						end
						return "thread"
					elseif type( Table[ 1 ] ) == "userdata" then
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "userdata" then
								if type( Table[ i ] ) == "function"
									and type( Table[ i ]( ) ) == "userdata" then
									goto is_userdata
								end
								return "mixed"
							end
							::is_userdata::
						end
						return "userdata"
					elseif type( Table[ 1 ] ) == "function" then
						Table[ 1 ] = Table[ 1 ]( )
						goto go_to_ini
						for i = 1, #Table do
							if type( Table[ i ] ) ~= "function" then
								return "mixed"
							end
						end
						return "function"
					end
					return "nil"
				end
				return "empty"
			end,

		},
		
		-- UTF8 sublibrary
		utf8 = {
			charrange = function( s, i )
				-- UTF8 character range at string codepoint
				if type( s ) ~= "string" or type( i ) ~= "number" or i < 1 or i > #s then
					error( "string and string index expected", 2 )
				end
				local byte = s:byte( i )
				return not byte and 0 or
						byte < 192 and 1 or
						byte < 224 and 2 or
						byte < 240 and 3 or
						byte < 248 and 4 or
						byte < 252 and 5 or
						6
			end,
			
			chars = function( s )
				-- Creates iterator through UTF8 characters
				if type( s ) ~= "string" then
					error( "string expected", 2 )
				end
				local char_i, s_pos, s_len = 0, 1, #s
				return function( )
					if s_pos <= s_len then
						local cur_pos = s_pos
						s_pos = s_pos + ke4.utf8.charrange( s, s_pos )
						if s_pos - 1 <= s_len then
							char_i = char_i + 1
							return char_i, s:sub( cur_pos, s_pos - 1 )
						end
					end
				end
			end,
			
			len = function( s )
				-- Get UTF8 characters number in string
				if type( s ) ~= "string" then
					error( "string expected", 2 )
				end
				local n = 0
				for _ in ke4.utf8.chars( s ) do
					n = n + 1
				end
				return n
			end
		},
		
		-- Math sublibrary
		math = {
			format2 = function( String )
				local str_mathf = String
				local str_mathformat
				if pcall( loadstring( format( [[
						return function( )
							local x = 5
							%s
							return x
						end
						]], str_mathf )
					) ) then
					local math_format_funct = loadstring( format( [[
						return function( )
							local x = 5
							%s
							return x
						end
						]], str_mathf )
					)( )
					if pcall( math_format_funct ) then
						str_mathformat = math_format_funct( )
						if str_mathformat then
							return str_mathformat
						end
						return str_mathf
					end
				end
				return str_mathf
			end, --!_G.ke4.math.format2( "x = x + 7" )!
		
			format = function( String, ... )
				--le da valores a un string de formato
				local Values = { ... }
				if ...
					and type( ... ) == "table" then
					Values = ...
				end
				if #Values == 0 then
					Values = { 1 }
				end --!_G.ke4.math.format( "frame_dur" )!
				local max_index = ke4.string.count( String, "%%[aAcdeEfgGioqsuxX]^*" ) --modos del string.format
				local str_mathf = format( String, unpack( ke4.table.replay( ceil( max_index / #Values ), Values ) ) )
				-----------------------------------------------------------------------------------------------------
				local str_mathformat
				str_mathf = str_mathf:gsub( "meta%.res_x", "xres" ):gsub( "meta%.res_y", "yres" )
				:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
				if pcall( loadstring( format( [[
						return function( )
							local xres, yres = aegisub.video_size( )
							if not xres then
								xres, yres = 1280, 720
							end
							local ratio = xres / 1280
							local msa = aegisub.ms_from_frame( 1 )
							local msb = aegisub.ms_from_frame( 101 )
							local frame_dur = 41.708
							if msb then
								frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
							end
							local pi, ln, log = math.pi, math.log, math.log10
							local sin, cos, tan = math.sin, math.cos, math.tan
							local abs, deg, rad = math.abs, math.deg, math.rad
							local asin, acos, atan = math.asin, math.acos, math.atan
							local sinh, cosh, tanh = math.sinh, math.cosh, math.tanh
							local rand, ceil, floor = math.random, math.ceil, math.floor
							local atan2, format = math.atan2, string.format
							local unpack = table.unpack or unpack
							return %s
						end
						]], str_mathf )
					) ) then
					local math_format_funct = loadstring( format( [[
						return function( )
							local xres, yres = aegisub.video_size( )
							if not xres then
								xres, yres = 1280, 720
							end
							local ratio = xres / 1280
							local msa = aegisub.ms_from_frame( 1 )
							local msb = aegisub.ms_from_frame( 101 )
							local frame_dur = 41.708
							if msb then
								frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
							end
							local pi, ln, log = math.pi, math.log, math.log10
							local sin, cos, tan = math.sin, math.cos, math.tan
							local abs, deg, rad = math.abs, math.deg, math.rad
							local asin, acos, atan = math.asin, math.acos, math.atan
							local sinh, cosh, tanh = math.sinh, math.cosh, math.tanh
							local rand, ceil, floor = math.random, math.ceil, math.floor
							local atan2, format = math.atan2, string.format
							local unpack = table.unpack or unpack
							return %s
						end
						]], str_mathf )
					)( )
					if pcall( math_format_funct ) then
						str_mathformat = math_format_funct( )
						if str_mathformat then
							return str_mathformat
						end
						str_mathf = str_mathf:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
						:gsub( "\"(&H%x+&)\"", "%1" )
						return str_mathf
					end --!_G.ke4.math.format( "%s + %s / 20", 5, 7 )!
				end
				str_mathf = str_mathf:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
				:gsub( "\"(&H%x+&)\"", "%1" )
				-----------------------------------------------------------------------------------------------------
				return str_mathf
			end,

			R = function( Rand_i, Rand_f, Step )
				--versión mejorada de la función math.random
				local rand_s = Step or 1
				local rand_i, rand_f = Rand_i, Rand_f
				------------------------------------
				local function math_R( Rand_i, Rand_f )
					local Offset_r = tonumber( tostring( os.time( ) ):sub( -3, -1 ) )
					local rand_i, rand_f = Rand_i, Rand_f
					-------------------------------------
					local function math_r( r_i, r_f, r_step )
						local rand_i, rand_f = r_i, r_f
						local rand_s = r_step or 1
						if rand_i == nil then
							return rand( )
						end
						if rand_f == nil then
							return rand( rand_i )
						end
						local r_ii = math.min( rand_i, rand_f )
						local r_ff = math.max( rand_i, rand_f )
						if rand_s == 0 then
							rand_s = 1
						end
						return rand( ke4.math.round( r_ii / rand_s ), ke4.math.round( r_ff / rand_s ) ) * rand_s
					end
					-------------------------------------
					if Rand_f == nil
						and Rand_i == nil then
						return ke4.math.round( math_r( ), 4 )
					end
					if Rand_f == nil
						and Rand_i then
						if Rand_i == 0 then
							return 0
						end
						rand_f = Rand_i
						rand_i = 1
					end
					local rand_i = ke4.math.round( rand_i )
					local rand_f = ke4.math.round( rand_f )
					local R_i = math.min( rand_i, rand_f )
					local R_f = math.max( rand_i, rand_f )
					return R_i + (math_r( R_i, R_f ) + Offset_r - 1) % (R_f - R_i + 1)
				end
				------------------------------------
				if rand_i == nil then
					return math_R( )
				end
				if rand_f == nil then
					return math_R( rand_i )
				end
				local Rand_ii = math.min( rand_i, rand_f )
				local Rand_ff = math.max( rand_i, rand_f )
				if rand_s == 0 then
					rand_s = 1
				end
				local Rand_v = Rand_ii + math_R( 0, (Rand_ff - Rand_ii) / rand_s ) * rand_s
				if Rand_v > Rand_ff then
					return Rand_ff
				end
				return Rand_v
			end,

			Rs = function( Rand_i, Rand_f, Step )
				--math.random con signos aleatorios incluidos
				return ke4.math.R( Rand_i, Rand_f, Step ) * (-1) ^ ke4.math.R( 2 )
			end,

			Rd = function( Rand_i, Rand_f, Step )
				--math.random redondeados a décimas
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				if Rnd_i then
					Rnd_i = Rnd_i * 10
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 10
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 10
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) / 10
			end,

			Rc = function( Rand_i, Rand_f, Step )
				--math.random redondeados a centésimas
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				if Rnd_i then
					Rnd_i = Rnd_i * 100
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 100
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 100
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) / 100
			end,

			Rm = function( Rand_i, Rand_f, Step )
				--math.random redondeados a milésimas
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				if Rnd_i then
					Rnd_i = Rnd_i * 1000
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 1000
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 1000
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) / 1000
			end,

			Rds = function( Rand_i, Rand_f, Step )
				--math.random redondeados a décimas con signo aleatorio
				return ke4.math.Rd( Rand_i, Rand_f, Step ) * (-1) ^ ke4.math.R( 2 )
			end,

			Rcs = function( Rand_i, Rand_f, Step )
				--math.random redondeados a centésimas con signo aleatorio
				return ke4.math.Rc( Rand_i, Rand_f, Step ) * (-1) ^ ke4.math.R( 2 )
			end,

			Rms = function( Rand_i, Rand_f, Step )
				--math.random redondeados a milésimas con signo aleatorio
				return ke4.math.Rm( Rand_i, Rand_f, Step ) * (-1) ^ ke4.math.R( 2 )
			end,

			Rr = function( Rand_i, Rand_f, Step )
				--math random ratio
				local rand_s = Step or 1
				local rand_i, rand_f = Rand_i, Rand_f
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if rand_i == nil then
					return ke4.math.R( )
				end
				if rand_f == nil then
					return ke4.math.R( rand_i * ratio )
				end
				return ke4.math.R( rand_i * ratio, rand_f * ratio, rand_s )
			end,

			Rsr = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * ratio * (-1) ^ ke4.math.R( 2 )
			end,
			
			Rdr = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if Rnd_i then
					Rnd_i = Rnd_i * 10
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 10
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 10
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * ratio / 10
			end,

			Rcr = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if Rnd_i then
					Rnd_i = Rnd_i * 100
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 100
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 100
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * ratio / 100
			end,

			Rmr = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if Rnd_i then
					Rnd_i = Rnd_i * 1000
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 1000
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 1000
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * ratio / 1000
			end,

			Rdrs = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if Rnd_i then
					Rnd_i = Rnd_i * 10
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 10
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 10
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * 0.1 * ratio * (-1) ^ ke4.math.R( 2 )
			end,

			Rcrs = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if Rnd_i then
					Rnd_i = Rnd_i * 100
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 100
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 100
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * 0.01 * ratio * (-1) ^ ke4.math.R( 2 )
			end,

			Rmrs = function( Rand_i, Rand_f, Step )
				local Rnd_i, Rnd_f, Rnd_s = Rand_i, Rand_f, Step
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				if Rnd_i then
					Rnd_i = Rnd_i * 1000
				end
				if Rnd_f then
					Rnd_f = Rnd_f * 1000
				end
				if Rnd_s then
					Rnd_s = Rnd_s * 1000
				end
				return ke4.math.R( Rnd_i, Rnd_f, Rnd_s ) * 0.001 * ratio * (-1) ^ ke4.math.R( 2 )
			end,

			Re = function( Table )
				--retorna un elemento al azar de la tabla ingresada
				local Table = Table or { 1 }
				local keys = { }
				for k, v in pairs( Table ) do
					table.insert( keys, k )
				end
				return Table[ keys[ ke4.math.R( #keys ) ] ]
			end,

			arc_curve = function( x, y, cx, cy, angle )
				-- Converts an arc to 1-4 cubic bezier curve( s )
				if type( x ) ~= "number" or type( y ) ~= "number" or type( cx ) ~= "number" or type( cy ) ~= "number" or type( angle ) ~= "number" or
					angle < -360 or angle > 360 then
					error("start & center point and valid angle (-360<=x<=360) expected", 2)
				end
				if angle ~= 0 then
					local kappa = 4 * (( 2 ) ^ 0.5 - 1) / 3
					local rx0, ry0, rx1, ry1, rx2, ry2, rx3, ry3, rx03, ry03 = x - cx, y - cy
					local cw = angle > 0 and 1 or -1
					if angle < 0 then
						angle = -angle
					end
					local curves, curves_n, angle_sum, cur_angle_pct = { }, 0, 0
					repeat
						cur_angle_pct = math.min( angle - angle_sum, 90 ) / 90
						rx3, ry3 = rotate2d( rx0, ry0, cw * 90 * cur_angle_pct )
						rx03, ry03 = rx3 - rx0, ry3 - ry0
						rx03, ry03 = ke4.math.stretch( rx03, ry03, 0, math.sqrt( ke4.math.distance2( rx03, ry03 ) ^ 2 / 2 ) * kappa )
						rx1, ry1 = rotate2d( rx03, ry03, cw * -45 * cur_angle_pct )
						rx1, ry1 = rx0 + rx1, ry0 + ry1
						rx2, ry2 = rotate2d( -rx03, -ry03, cw * 45 * cur_angle_pct )
						rx2, ry2 = rx3 + rx2, ry3 + ry2
						curves[ curves_n + 1 ], curves[ curves_n + 2 ], curves[ curves_n + 3 ], curves[ curves_n + 4 ],
						curves[ curves_n + 5 ], curves[ curves_n + 6 ], curves[ curves_n + 7 ], curves[ curves_n + 8 ] =
						cx + rx0, cy + ry0, cx + rx1, cy + ry1, cx + rx2, cy + ry2, cx + rx3, cy + ry3
						curves_n = curves_n + 8
						rx0, ry0 = rx3, ry3
						angle_sum = angle_sum + 90
					until angle_sum >= angle
					return unpack( curves )
				end
			end,
			
			angle = function( px1, py1, px2, py2 )
				--mide el ángulo entre dos puntos o entre el origen (0, 0) y un punto
				local angle, x1, x2, y1, y2 = 0, 0, 0, 0, 0
				if type( px1 ) ~= "table"
					and px2 == nil then
					x1 = 0
					y1 = 0
					x2 = px1
					y2 = py1
				elseif type( px1 ) ~= "table"
					and type( px2 ) ~= "table" then
					x1 = px1
					y1 = py1
					x2 = px2
					y2 = py2
				elseif type( px1 ) ~= "table"
					and type( px2 ) == "table" then
					x1 = px1
					y1 = py1
					x2 = px2[ 1 ]
					y2 = px2[ 2 ]
				elseif type( px1 ) == "table"
					and py1 == nil then
					x1 = 0
					y1 = 0
					x2 = px1[ 1 ]
					y2 = px1[ 2 ]
				elseif type( px1 ) == "table"
					and type( py1 ) == "table" then
					x1 = px1[ 1 ]
					y1 = px1[ 2 ]
					x2 = py1[ 1 ]
					y2 = py1[ 2 ]
				elseif type( px1 ) == "table"
					and type( py1 ) ~= "table" then
					x1 = px1[ 1 ]
					y1 = px1[ 2 ]
					x2 = py1
					y2 = px2
				end
				local Ang = deg( atan( (y2 - y1) / (x2 - x1) ) )
				if x2 > x1
					and y2 > y1 then
					angle = 360 - Ang
				elseif x2 > x1
					and y2 < y1 then
					angle = -Ang
				elseif x2 < x1
					and y2 < y1 then
					angle = 180 - Ang
				elseif x2 < x1
					and y2 > y1 then
					angle = 180 - Ang
				elseif x2 > x1
					and y2 == y1 then
					angle = 0
				elseif x2 < x1
					and y2 == y1 then
					angle = 180
				elseif x2 == x1
					and y2 < y1 then
					angle = 90
				elseif x2 == x1
					and y2 > y1 then
					angle = 270
				elseif x2 == x1
					and y2 == y1 then
					angle = 0
				end
				return ke4.math.round( angle, 3 )
			end,
			
			create_matrix = function( )
				-- Creates 3d matrix
				local matrix = { 1, 0, 0, 0,
								 0, 1, 0, 0,
								 0, 0, 1, 0,
								 0, 0, 0, 1 }
				local obj
				obj = {
					get_data = function( )
						return ke4.table.copy( matrix )
					end,
					
					set_data = function( new_matrix )
						if type( new_matrix ) ~= "table" or #new_matrix ~= 16 then
							error( "4x4 matrix expected", 2 )
						end
						for _, value in ipairs( new_matrix ) do
							if type( value ) ~= "number" then
								error( "matrix must contain only numbers", 2 )
							end
						end
						matrix = ke4.table.copy( new_matrix )
						return obj
					end,
					
					identity = function( )
						matrix[ 1 ] = 1
						matrix[ 2 ] = 0
						matrix[ 3 ] = 0
						matrix[ 4 ] = 0
						matrix[ 5 ] = 0
						matrix[ 6 ] = 1
						matrix[ 7 ] = 0
						matrix[ 8 ] = 0
						matrix[ 9 ] = 0
						matrix[ 10 ] = 0
						matrix[ 11 ] = 1
						matrix[ 12 ] = 0
						matrix[ 13 ] = 0
						matrix[ 14 ] = 0
						matrix[ 15 ] = 0
						matrix[ 16 ] = 1
						return obj
					end,
					
					multiply = function( matrix2 )
						if type( matrix2 ) ~= "table" or #matrix2 ~= 16 then
							error( "4x4 matrix expected", 2 )
						end
						for _, value in ipairs( matrix2 ) do
							if type( value ) ~= "number" then
								error( "matrix must contain only numbers", 2 )
							end
						end
						local new_matrix = { 0, 0, 0, 0,
											 0, 0, 0, 0,
											 0, 0, 0, 0,
											 0, 0, 0, 0 }
						for i = 1, 16 do
							for j = 0, 3 do
								new_matrix[ i ] = new_matrix[ i ] + matrix[ 1 + (i - 1) % 4 + j * 4 ] * matrix2[ 1 + floor( (i - 1) / 4 ) * 4 + j ]
							end
						end
						matrix = new_matrix
						return obj
					end,
					
					translate = function( x, y, z )
						-- Applies translation to matrix
						if type( x ) ~= "number" or type( y ) ~= "number" or type( z ) ~= "number" then
							error( "3 translation values expected", 2 )
						end
						obj.multiply( { 1, 0, 0, 0,
										0, 1, 0, 0,
										0, 0, 1, 0,
										x, y, z, 1 } )
						-- Return this object
						return obj
					end,
					
					scale = function( x, y, z )
						-- Applies scale to matrix
						if type( x ) ~= "number" or type( y ) ~= "number" or type( z ) ~= "number" then
							error( "3 scale factors expected", 2 )
						end
						obj.multiply( { x, 0, 0, 0,
										0, y, 0, 0,
										0, 0, z, 0,
										0, 0, 0, 1 } )
						return obj
					end,
					
					rotate = function( axis, angle )
						-- Applies rotation to matrix
						if ( axis ~= "x" and axis ~= "y" and axis ~= "z" ) or type( angle ) ~= "number" then
							error( "axis (as string) and angle (in degree) expected", 2 )
						end
						angle = rad( angle )
						if axis == "x" then
							obj.multiply( { 1, 0, 0, 0,
											0, cos( angle ), -sin( angle ), 0,
											0, sin( angle ), cos( angle ), 0,
											0, 0, 0, 1 } )
						elseif axis == "y" then
							obj.multiply( { cos( angle ), 0, sin( angle ), 0,
											0, 1, 0, 0,
											-sin( angle ), 0, cos( angle ), 0,
											0, 0, 0, 1 } )
						else	-- axis == "z"
							obj.multiply( { cos( angle ), -sin( angle ), 0, 0,
											sin( angle ), cos( angle ), 0, 0,
											0, 0, 1, 0,
											0, 0, 0, 1 } )
						end
						return obj
					end,
					
					inverse = function( )
						-- Inverses matrix
						local inv_matrix = {
							matrix[ 6 ] * matrix[ 11 ] * matrix[ 16 ] - matrix[ 6 ] * matrix[ 15 ] * matrix[ 12 ] - matrix[ 7 ] * matrix[ 10 ] * matrix[ 16 ] + matrix[ 7 ] * matrix[ 14 ] * matrix[ 12 ] +matrix[ 8 ] * matrix[ 10 ] * matrix[ 15 ] - matrix[ 8 ] * matrix[ 14 ] * matrix[ 11 ],
							-matrix[ 2 ] * matrix[ 11 ] * matrix[ 16 ] + matrix[ 2 ] * matrix[ 15 ] * matrix[ 12 ] + matrix[ 3 ] * matrix[ 10 ] * matrix[ 16 ] - matrix[ 3 ] * matrix[ 14 ] * matrix[ 12 ] - matrix[ 4 ] * matrix[ 10 ] * matrix[ 15 ] + matrix[ 4 ] * matrix[ 14 ] * matrix[ 11 ],
							matrix[ 2 ] * matrix[ 7 ] * matrix[ 16 ] - matrix[ 2 ] * matrix[ 15 ] * matrix[ 8 ] - matrix[ 3 ] * matrix[ 6 ] * matrix[ 16 ] + matrix[ 3 ] * matrix[ 14 ] * matrix[ 8 ] + matrix[ 4 ] * matrix[ 6 ] * matrix[ 15 ] - matrix[ 4 ] * matrix[ 14 ] * matrix[ 7 ],
							-matrix[ 2 ] * matrix[ 7 ] * matrix[ 12 ] + matrix[ 2 ] * matrix[ 11 ] * matrix[ 8 ] +matrix[ 3 ] * matrix[ 6 ] * matrix[ 12 ] - matrix[ 3 ] * matrix[ 10 ] * matrix[ 8 ] - matrix[ 4 ] * matrix[ 6 ] * matrix[ 11 ] + matrix[ 4 ] * matrix[ 10 ] * matrix[ 7 ],
							-matrix[ 5 ] * matrix[ 11 ] * matrix[ 16 ] + matrix[ 5 ] * matrix[ 15 ] * matrix[ 12 ] + matrix[ 7 ] * matrix[ 9 ] * matrix[ 16 ] - matrix[ 7 ] * matrix[ 13 ] * matrix[ 12 ] - matrix[ 8 ] * matrix[ 9 ] * matrix[ 15 ] + matrix[ 8 ] * matrix[ 13 ] * matrix[ 11 ],
							matrix[ 1 ] * matrix[ 11 ] * matrix[ 16 ] - matrix[ 1 ] * matrix[ 15 ] * matrix[ 12 ] - matrix[ 3 ] * matrix[ 9 ] * matrix[ 16 ] + matrix[ 3 ] * matrix[ 13 ] * matrix[ 12 ] + matrix[ 4 ] * matrix[ 9 ] * matrix[ 15 ] - matrix[ 4 ] * matrix[ 13 ] * matrix[ 11 ],
							-matrix[ 1 ] * matrix[ 7 ] * matrix[ 16 ] + matrix[ 1 ] * matrix[ 15 ] * matrix[ 8 ] + matrix[ 3 ] * matrix[ 5 ] * matrix[ 16 ] - matrix[ 3 ] * matrix[ 13 ] * matrix[ 8 ] - matrix[ 4 ] * matrix[ 5 ] * matrix[ 15 ] + matrix[ 4 ] * matrix[ 13 ] * matrix[ 7 ],
							matrix[ 1 ] * matrix[ 7 ] * matrix[ 12 ] - matrix[ 1 ] * matrix[ 11 ] * matrix[ 8 ] - matrix[ 3 ] * matrix[ 5 ] * matrix[ 12 ] + matrix[ 3 ] * matrix[ 9 ] * matrix[ 8 ] + matrix[ 4 ] * matrix[ 5 ] * matrix[ 11 ] - matrix[ 4 ] * matrix[ 9 ] * matrix[ 7 ],
							matrix[ 5 ] * matrix[ 10 ] * matrix[ 16 ] - matrix[ 5 ] * matrix[ 14 ] * matrix[ 12 ] - matrix[ 6 ] * matrix[ 9 ] * matrix[ 16 ] + matrix[ 6 ] * matrix[ 13 ] * matrix[ 12 ] + matrix[ 8 ] * matrix[ 9 ] * matrix[ 14 ] - matrix[ 8 ] * matrix[ 13 ] * matrix[ 10 ],
							-matrix[ 1 ] * matrix[ 10 ] * matrix[ 16 ] + matrix[ 1 ] * matrix[ 14 ] * matrix[ 12 ] + matrix[ 2 ] * matrix[ 9 ] * matrix[ 16 ] - matrix[ 2 ] * matrix[ 13 ] * matrix[ 12 ] - matrix[ 4 ] * matrix[ 9 ] * matrix[ 14 ] + matrix[ 4 ] * matrix[ 13 ] * matrix[ 10 ],
							matrix[ 1 ] * matrix[ 6 ] * matrix[ 16 ] - matrix[ 1 ] * matrix[ 14 ] * matrix[ 8 ] - matrix[ 2 ] * matrix[ 5 ] * matrix[ 16 ] + matrix[ 2 ] * matrix[ 13 ] * matrix[ 8 ] + matrix[ 4 ] * matrix[ 5 ] * matrix[ 14 ] - matrix[ 4 ] * matrix[ 13 ] * matrix[ 6 ],
							-matrix[ 1 ] * matrix[ 6 ] * matrix[ 12 ] + matrix[ 1 ] * matrix[ 10 ] * matrix[ 8 ] + matrix[ 2 ] * matrix[ 5 ] * matrix[ 12 ] - matrix[ 2 ] * matrix[ 9 ] * matrix[ 8 ] - matrix[ 4 ] * matrix[ 5 ] * matrix[ 10 ] + matrix[ 4 ] * matrix[ 9 ] * matrix[ 6 ],
							-matrix[ 5 ] * matrix[ 10 ] * matrix[ 15 ] + matrix[ 5 ] * matrix[ 14 ] * matrix[ 11 ] + matrix[ 6 ] * matrix[ 9 ] * matrix[ 15 ] - matrix[ 6 ] * matrix[ 13 ] * matrix[ 11 ] - matrix[ 7 ] * matrix[ 9 ] * matrix[ 14 ] + matrix[ 7 ] * matrix[ 13 ] * matrix[ 10 ],
							matrix[ 1 ] * matrix[ 10 ] * matrix[ 15 ] - matrix[ 1 ] * matrix[ 14 ] * matrix[ 11 ] - matrix[ 2 ] * matrix[ 9 ] * matrix[ 15 ] + matrix[ 2 ] * matrix[ 13 ] * matrix[ 11 ] + matrix[ 3 ] * matrix[ 9 ] * matrix[ 14 ] - matrix[ 3 ] * matrix[ 13 ] * matrix[ 10 ],
							-matrix[ 1 ] * matrix[ 6 ] * matrix[ 15 ] + matrix[ 1 ] * matrix[ 14 ] * matrix[ 7 ] + matrix[ 2 ] * matrix[ 5 ] * matrix[ 15 ] - matrix[ 2 ] * matrix[ 13 ] * matrix[ 7 ] - matrix[ 3 ] * matrix[ 5 ] * matrix[ 14 ] + matrix[ 3 ] * matrix[ 13 ] * matrix[ 6 ],
							matrix[ 1 ] * matrix[ 6 ] * matrix[ 11 ] - matrix[ 1 ] * matrix[ 10 ] * matrix[ 7 ] - matrix[ 2 ] * matrix[ 5 ] * matrix[ 11 ] + matrix[ 2 ] * matrix[ 9 ] * matrix[ 7 ] + matrix[ 3 ] * matrix[ 5 ] * matrix[ 10 ] - matrix[ 3 ] * matrix[ 9 ] * matrix[ 6 ]
						}
						local det = matrix[ 1 ] * inv_matrix[ 1 ] + matrix[ 5 ] * inv_matrix[ 2 ] + matrix[ 9 ] * inv_matrix[ 3 ] + matrix[ 13 ] * inv_matrix[ 4 ]
						if det ~= 0 then
							det = 1 / det
							for i = 1, 16 do
								matrix[ i ] = inv_matrix[ i ] * det
							end
							return obj
						end
					end,
					
					transform = function( x, y, z, w )
						-- Applies matrix to point
						if type( x ) ~= "number" or type( y ) ~= "number" or type( z ) ~= "number" or (w ~= nil and type( w ) ~= "number") then
							error( "point (3 or 4 numbers) expected", 2 )
						end
						if not w then
							w = 1
						end
						return x * matrix[ 1 ] + y * matrix[ 5 ] + z * matrix[ 9 ] + w * matrix[ 13 ],
								x * matrix[ 2 ] + y * matrix[ 6 ] + z * matrix[ 10 ] + w * matrix[ 14 ],
								x * matrix[ 3 ] + y * matrix[ 7 ] + z * matrix[ 11 ] + w * matrix[ 15 ],
								x * matrix[ 4 ] + y * matrix[ 8 ] + z * matrix[ 12 ] + w * matrix[ 16 ]
					end
				}
				return obj
			end,
			
			degree = function( x1, y1, z1, x2, y2, z2 )
				-- Degree between two 3d vectors
				if type( x1 ) ~= "number" or type( y1 ) ~= "number" or type( z1 ) ~= "number" or
					type( x2 ) ~= "number" or type( y2 ) ~= "number" or type( z2 ) ~= "number" then
					error( "2 vectors (as 6 numbers) expected", 2 )
				end
				local degree = deg(
						acos(
							(x1 * x2 + y1 * y2 + z1 * z2) /
							( ke4.math.distance2( x1, y1, z1 ) * ke4.math.distance2( x2, y2, z2 ))
						)
				)
				return (x1 * y2 - y1 * x2) < 0 and -degree or degree
			end,
			
			polar = function( angle, radius, Return )
				--retorna las coordenadas del punto ubicado con el ángulo y radio dados, respecto al origen
				local angle = angle or 0
				local radius = radius or 0
				local Px = ke4.math.round(  radius * cos( rad( angle ) ), 3 )
				local Py = ke4.math.round( -radius * sin( rad( angle ) ), 3 )
				if Return == "x" then
					return Px
				end
				if Return == "y" then
					return Py
				end
				return Px, Py
			end,
			
			distance = function( px1, py1, px2, py2 )
				--mide la distancia entre dos puntos o entre un punto y el origen (0, 0)
				local x1, x2, y1, y2 = 0, 0, 0, 0
				if type( px1 ) ~= "table"
					and px2 == nil then				--math.distance( Px, Py )
					x1 = 0
					y1 = 0
					x2 = px1
					y2 = py1
				elseif type( px1 ) ~= "table"
					and type( px2 ) ~= "table" then	--math.distance( Px1, Py1, Px2, Py2 )
					x1 = px1
					y1 = py1
					x2 = px2
					y2 = py2
				elseif type( px1 ) ~= "table"
					and type( px2 ) == "table" then	--math.distance( Px1, Py1, { Px2, Py2 } )
					x1 = px1
					y1 = py1
					x2 = px2[ 1 ]
					y2 = px2[ 2 ]
				elseif type( px1 ) == "table"
					and py1 == nil then				--math.distance( { Px, Py } )
					x1 = 0
					y1 = 0
					x2 = px1[ 1 ]
					y2 = px1[ 2 ]
				elseif type( px1 ) == "table"
					and type( py1 ) == "table" then	--math.distance( { Px1, Py1 }, { Px2, Py2 } )
					x1 = px1[ 1 ]
					y1 = px1[ 2 ]
					x2 = py1[ 1 ]
					y2 = py1[ 2 ]
				elseif type( px1 ) == "table"
					and type( py1 ) ~= "table" then	--math.distance( { Px1, Py1 }, Px2, Py2 )
					x1 = px1[ 1 ]
					y1 = px1[ 2 ]
					x2 = py1
					y2 = px2
				end
				return ke4.math.round( ((x2 - x1) ^ 2 + (y2 - y1) ^ 2) ^ 0.5, 3 )
			end,
			
			distance2 = function( x, y, z )
				-- Length of vector
				if type( x ) ~= "number" or type( y ) ~= "number" or z ~= nil and type( z ) ~= "number" then
					error( "one vector (2 or 3 numbers) expected", 2 )
				end
				return z and (x * x + y * y + z * z) ^ 0.5 or (x * x + y * y) ^ 0.5
			end,
			
			intersect = function( x1, y1, x2, y2, x3, y3, x4, y4 )
				--retorna el punto en donde se intersectan las dos rectas definidas en 4 puntos
				local Ec1, Ec2 = { }, { }
				local im1 = (y2 - y1) / (x2 - x1)
				Ec1[ 1 ] = -im1
				Ec1[ 2 ] = 1
				Ec1[ 3 ] = y1 - im1 * x1
				if x1 == x2 then
					im1 = math.huge
					Ec1[ 1 ] = 1
					Ec1[ 2 ] = 0
					Ec1[ 3 ] = x1
				end
				local im2 = (y4 - y3) / (x4 - x3)
				Ec2[ 1 ] = -im2
				Ec2[ 2 ] = 1
				Ec2[ 3 ] = y3 - im2 * x3
				if x3 == x4 then
					im2 = math.huge
					Ec2[ 1 ] = 1
					Ec2[ 2 ] = 0
					Ec2[ 3 ] = x3
				end
				if im1 == im2 then
					return "las rectas son paralelas"
				end
				local DetI = Ec1[ 1 ] * Ec2[ 2 ] - Ec1[ 2 ] * Ec2[ 1 ]
				local DetX = Ec1[ 3 ] * Ec2[ 2 ] - Ec1[ 2 ] * Ec2[ 3 ]
				local DetY = Ec1[ 1 ] * Ec2[ 3 ] - Ec1[ 3 ] * Ec2[ 1 ]
				local ix = ke4.math.round( DetX / DetI, 3 )
				local iy = ke4.math.round( DetY / DetI, 3 )
				return ix, iy
			end,
			
			line_intersect = function( x0, y0, x1, y1, x2, y2, x3, y3, strict )
				if type( x0 ) ~= "number" or type( y0 ) ~= "number" or type( x1 ) ~= "number" or type( y1 ) ~= "number" or
					type( x2 ) ~= "number" or type( y2 ) ~= "number" or type( x3 ) ~= "number" or type( y3 ) ~= "number" or
					strict ~= nil and type( strict ) ~= "boolean" then
					error( "two lines and optional strictness flag expected", 2 )
				end
				local x10, y10, x32, y32 = x0 - x1, y0 - y1, x2 - x3, y2 - y3
				if x10 == 0 and y10 == 0 or x32 == 0 and y32 == 0 then
					error( "lines mustn't have zero length", 2 )
				end
				local det = x10 * y32 - y10 * x32
				if det ~= 0 then
					local pre, post = (x0 * y1 - y0 * x1), (x2 * y3 - y2 * x3)
					local ix, iy = (pre * x32 - x10 * post) / det, (pre * y32 - y10 * post) / det
					if strict then
						local s, t = x10 ~= 0 and (ix - x1) / x10 or (iy - y1) / y10, x32 ~= 0 and (ix - x3) / x32 or (iy - y3) / y32
						if s < 0 or s > 1 or t < 0 or t > 1 then
							return 1 / 0	-- infinite
						end
					end
					return ix, iy
				end
			end,
			
			factk = function( n )
				--calcula el factorial de un número
				local k_factk = 1
				if n > 1 then
					for i = 2, n do
						k_factk = k_factk * i
					end
				end
				return k_factk
			end,
			
			bernstein = function( i, n, t )
				--algoritmo para generar los puntos de una curva Bezier
				return (ke4.math.factk( n ) / (ke4.math.factk( i ) * ke4.math.factk( n - i ))) * (t ^ i) * ((1 - t) ^ (n - i))
			end,
			
			confi_bezier = function( n, x, y, t, Return )
				--configura los puntos de una curva Bezier
				local Px, Py = x, y
				local pos_x, pos_y = 0, 0
				if y == nil then
					Px, Py = { }, { }
					if type( x ) == "string" then
						local coor = { }
						for c in x:gmatch( "%-?%d+[%.%d]*" ) do
							table.insert( coor, tonumber( c ) )
						end
						for i = 1, #coor / 2 do
							Px[ i ] = coor[ 2 * i - 1 ]
							Py[ i ] = coor[ 2 * i - 0 ]
						end
					elseif type( x ) == "table" then
						for i = 1, #x / 2 do
							Px[ i ] = x[ 2 * i - 1 ]
							Py[ i ] = x[ 2 * i - 0 ]
						end
					end
				end
				local n = n or #Px
				for i = 1, n do
					pos_x = pos_x + Px[ i ] * ke4.math.bernstein( i - 1, n - 1, t )
					pos_y = pos_y + Py[ i ] * ke4.math.bernstein( i - 1, n - 1, t )
				end
				if Return == "x" then
					return pos_x
				end
				if Return == "y" then
					return pos_y
				end
				return pos_x, pos_y
			end,
			
			length_bezier = function( ... )
				--longitud de una curva Bezier a partir de sus puntos
				local Px, Py, Bx, By = { }, { }, { }, { }
				local nN, shape_bezier = 1024, ""
				local Blength, coor = 0, { ... }
				if type( ... ) == "table" then
					coor = ...
				elseif type( ... ) == "string" then
					coor = { }
					shape_bezier = ke4.shape.ASSDraw3( ... )
					for c in shape_bezier:gmatch( "%-?%d+[%.%d]*" ) do
						table.insert( coor, tonumber( c ) )
					end
				end
				if #coor == 4 then
					Blength = ke4.math.distance( coor[ 1 ], coor[ 2 ], coor[ 3 ], coor[ 4 ] )
				else
					for i = 1, #coor / 2 do
						Px[ i ] = coor[ 2 * i - 1 ]
						Py[ i ] = coor[ 2 * i - 0 ]
					end
					for i = 1, nN do
						Bx[ i ], By[ i ] = ke4.math.confi_bezier( #Px, Px, Py, (i - 1) / (nN - 1) )
					end
					for i = 2, nN do
						Blength = Blength + ke4.math.distance( Bx[ i ], By[ i ], Bx[ i - 1 ], By[ i - 1 ] )
					end
				end
				return Blength
			end,
			
			angle_bezier = function( points, t )
				--ángulo de la derivada en un punto de una curva Bezier
				local coor, Px, Py, shape_bezier = { }, { }, { }, ""
				local Angle, Pdx, Pdy = recall.mA_bezier, 0, 0
				local t = t or 1
				--if j == 1 then
					if type( points ) == "table" then
						coor = points
					elseif type( points ) == "string" then
						shape_bezier = ke4.shape.ASSDraw3( points )
						for c in shape_bezier:gmatch( "%-?%d+[%.%d]*" ) do
							table.insert( coor, tonumber( c ) )
						end
					end
					for i = 1, #coor / 2 do
						Px[i] = coor[ 2 * i - 1 ]
						Py[i] = coor[ 2 * i - 0 ]
					end
					if #Px == 2 then
						Angle = remember( "mA_bezier", deg( atan( -(Py[ 2 ] - Py[ 1 ]) / (Px[ 2 ] - Px[ 1 ]) ) ) )
					else
						Pdx = -3 * (Px[ 1 ] - Px[ 2 ]) * (1 - t) ^ 2 - 6 * (Px[ 2 ] - Px[ 3 ]) * t * (1 - t) - 3 * (Px[ 3 ] - Px[ 4 ]) * t ^ 2
						Pdy = -3 * (Py[ 1 ] - Py[ 2 ]) * (1 - t) ^ 2 - 6 * (Py[ 2 ] - Py[ 3 ]) * t * (1 - t) - 3 * (Py[ 3 ] - Py[ 4 ]) * t ^ 2
						Angle = remember( "mA_bezier", deg( atan( -Pdy / Pdx ) ) )
					end
				--end
				return ke4.math.round( Angle, 3 )
			end,
			
			point = function( n, x_range, y_range, start_x, start_y, end_x, end_y )
				--genera puntos aleatorios con parámetros indicados (función Bezier)
				local Points = { }
				local Cn = n or 4
				local Rx = x_range or 2.5 * 56 --l.fontsize
				local Ry = y_range or 2.5 * 56 --l.fontsize
				local Sx = start_x or ke4.math.Rs( Rx )
				local Sy = start_y or ke4.math.Rs( Ry )
				local Ex = end_x or 0
				local Ey = end_y or 0
				for i = 3, 2 * Cn - 2, 2 do
					Points[ i + 0 ] = ke4.math.Rs( Rx )
					Points[ i + 1 ] = ke4.math.Rs( Ry )
				end
				Points[ 1 ] = Sx
				Points[ 2 ] = Sy
				Points[ 2 * Cn - 1 ] = Ex
				Points[ 2 * Cn - 0 ] = Ey
				return Points
			end,
			
			ortho = function( x1, y1, z1, x2, y2, z2 )
				-- Get orthogonal vector of 2 given vectors
				if type( x1 ) ~= "number" or type( y1 ) ~= "number" or type( z1 ) ~= "number" or
					type( x2 ) ~= "number" or type( y2 ) ~= "number" or type( z2 ) ~= "number" then
					error( "2 vectors (as 6 numbers) expected", 2 )
				end
				return y1 * z2 - z1 * y2, z1 * x2 - x1 * z2, x1 * y2 - y1 * x2
			end,
			
			randomsteps = function( min, max, step )
				-- Generates a random number in given range with specific item distance
				if type( min ) ~= "number" or type( max ) ~= "number" or type( step ) ~= "number" or max < min or step <= 0 then
					error( "minimal, maximal and step number expected", 2 )
				end
				return math.min( min + math.random( 0, ceil( (max - min) / step ) ) * step, max )
			end,
			
			round = function( number_or_table, decimal )
				-- redondea un número o una tabla de números a la
				-- cantidad de decimales indicados o al entero más cercano
				local decimal = decimal or 0
				local Num_round = number_or_table
				local multiple, Table_round = 10 ^ decimal, { }
				if type( Num_round ) == "number"
					or type( tonumber( Num_round ) ) == "number" then
					return floor( tonumber( Num_round ) * multiple + 0.5 ) / multiple
				elseif type( Num_round ) == "table" then
					for i, v in pairs( Num_round ) do
						Table_round[ i ] = v
						if type( v ) == "number"
							or type( tonumber( v ) ) == "number" then
							Table_round[ i ] = floor( tonumber( v ) * multiple + 0.5 ) / multiple
						end
					end
					return Table_round
				end
				return Num_round
			end,
			
			stretch = function( x, y, z, length )
				-- Scales vector to given length
				if type( x ) ~= "number" or type( y ) ~= "number" or type( z ) ~= "number" or type( length ) ~= "number" then
					error( "vector (3d) and length expected", 2 )
				end
				local cur_length = ke4.math.distance2( x, y, z )
				if cur_length == 0 then
					return 0, 0, 0
				else
					local factor = length / cur_length
					return x * factor, y * factor, z * factor
				end
			end,
			
			i = function( counter, A, B, C, F )
				local idx = counter
				local A = A or 1
				local B = B or 1
				local C = C or 1
				local D = A - B + 1
				local E = B - A + 1
				local F = F or 1
				local algorithms = {
					[ 01 ] = (-1) ^ (idx + 1),												-->( +,- )
					[ 02 ] = (-1) ^ idx,													-->( -,+ )
					[ 03 ] = (-1) ^ (ceil( idx / A ) + 1),									-->( ++,-- ) A-veces
					[ 04 ] = (-1) ^ ceil( idx / A ),										-->( --,++ ) A-veces
					[ 05 ] = (1 + (-1) ^ idx) / 2,											-->( 0,1 )
					[ 06 ] = (1 - (-1) ^ idx) / 2,											-->( 1,0 )
					[ 07 ] = (1 + (-1) ^ ceil( idx / A )) / 2,								-->( 00,11 ) A-veces
					[ 08 ] = (1 - (-1) ^ ceil( idx / A )) / 2,								-->( 11,00 ) A-veces
					[ 09 ] = (A / 2) * (1 + (-1) ^ idx ),									-->( 0,A )
					[ 10 ] = (A / 2) * (1 - (-1) ^ idx ),									-->( A,0 )
					[ 11 ] = (A / 2) * (1 + (-1) ^ ceil( idx / B ) ),						-->( 00,AA ) B-veces
					[ 12 ] = (A / 2) * (1 - (-1) ^ ceil( idx / B ) ),						-->( AA,00 ) B-veces
					[ 13 ] = A + ((B - A) / 2) * (1 + (-1) ^ idx ),							-->( A,B )
					[ 14 ] = A + ((B - A) / 2) * (1 + (-1) ^ ceil( idx / C ) ),				-->( AA,BB ) C-veces
					[ 15 ] = B * ceil( idx / A ),											-->( A,mB )  A-veces los múltiplos de B
					[ 16 ] = A - A * ceil( idx / A ) + idx,									-->( 1-->A )
					[ 17 ] = A * ceil( idx / A ) - idx + 1,									-->( A-->1 )
					[ 18 ] = (A - A * ceil( idx / A ) + idx) * (-1) ^ (ceil( idx / A ) + 1),-->( 1-->A,-1-->-A )
					[ 19 ] = (A - 1 - (A - 1) * ceil( idx / (A - 1) ) + idx) * (-1) ^ (ceil( idx / (A - 1) ) + 1) + (A + 1) * (1 + (-1) ^ ceil( idx / (A - 1) )) / 2,
					--[ 19 ]																-->( 1-->A-->1 )
					[ 20 ] = A + 1 - ((A - 1 - (A - 1) * ceil( idx / (A - 1) ) + idx) * (-1) ^ (ceil( idx / (A - 1) ) + 1) + (A + 1) * (1 + (-1) ^ ceil( idx / (A - 1) )) / 2),
					--[ 20 ]																-->( A-->1-->A )
					[ 21 ] = (( A > B )
						and B + D - ((D - 1 - (D - 1) * ceil( idx / (D - 1) ) + idx) * (-1) ^ (ceil( idx / (D - 1) ) + 1) + (D + 1) * (1 + (-1) ^ ceil( idx / (D - 1) )) / 2)
						or  A + (E - 1 - (E - 1) * ceil( idx / (E - 1) ) + idx) * (-1) ^ (ceil( idx / (E - 1) ) + 1) + (E + 1) * (1 + (-1) ^ ceil( idx / (E - 1) )) / 2 - 1
					),--[ 21 ]																-->( A-->B-->A )
					[ 22 ] = 1 - floor( 1 / idx ),											-->( 0,11 ) primer 0 y el resto 1
					[ 23 ] = floor( 1 / idx ),												-->( 1,00 ) primer 1 y el resto 0
					[ 24 ] = A * (1 - floor( 1 / idx )),									-->( 0,AA ) primer 0 y el resto A
					[ 25 ] = A * floor( 1 / idx ),											-->( A,00 ) primero A y el resto 0
					[ 26 ] = A * floor( 1 / idx ) + B * (1 - floor( 1 / idx )),				-->( A,BB" ) primero A y el resto B
					[ 27 ] = 1 - floor( (A * ceil( idx / A ) - idx + 1) / A ),				-->( 0<->11 ) primer 0 y (A-1)veces 1
					[ 28 ] = floor( (A - A * ceil( idx / A ) + idx) / A ),					-->( 00<->1 ) (A-1)veces 0 el un 1
					[ 29 ] = floor( (A * ceil( idx / A ) - idx + 1) / A ),					-->( 1<->00 ) primer 1 y (A-1)veces 0
					[ 30 ] = 1 - floor( (A - A * ceil( idx / A ) + idx) / A ),				-->( 11<->0 ) (A-1)veces 1 el un 0
					[ 31 ] = A * (1 - floor( (B * ceil( idx / B ) - idx + 1) / B )),		-->( 0<->AA ) primer 0 y (B-1)veces A
					[ 32 ] = A * floor( (B - B * ceil( idx / B ) + idx) / B ),				-->( 00<->A ) (B-1)veces 0 y un A
					[ 33 ] = A * floor( (B * ceil( idx / B ) - idx + 1) / B ),				-->( A<->00 ) primer A y (B-1)veces 0
					[ 34 ] = A * (1 - floor( (B - B * ceil( idx / B ) + idx) / B )),		-->( AA<->0 ) (B-1)veces A y un 0
					[ 35 ] = A * floor( (C * ceil( idx / C ) - idx + 1) / C ) + B * (1 - floor( (C * ceil( idx / C ) - idx + 1) / C )),	-->( A<->BB ) primer A y (C-1)veces B
					[ 36 ] = A * (1 - floor( (C - C * ceil( idx / C ) + idx) / C )) + B * floor( (C - C * ceil( idx / C ) + idx) / C ),	-->( AA<->B ) (C-1)veces A y un B
					[ 37 ] = floor( (idx - 1) / A ) + 1,									-->( N,n ) los Naturales n-veces cada uno
					------------------------------------------------------------------------------------------
					
					[ "+,-" ]			= (-1) ^ (idx + 1),
					[ "-,+" ]			= (-1) ^ idx,
					[ "++,--" ]			= (-1) ^ (ceil( idx / A ) + 1),
					[ "--,++" ]			= (-1) ^ ceil( idx / A ),
					[ "0,1" ]			= (1 + (-1) ^ idx) / 2,
					[ "1,0" ]			= (1 - (-1) ^ idx) / 2,
					[ "00,11" ]			= (1 + (-1) ^ ceil( idx / A )) / 2,
					[ "11,00" ]			= (1 - (-1) ^ ceil( idx / A )) / 2,
					[ "0,A" ]			= (A / 2) * (1 + (-1) ^ idx ),
					[ "A,0" ]			= (A / 2) * (1 - (-1) ^ idx ),
					[ "00,AA" ]			= (A / 2) * (1 + (-1) ^ ceil( idx / B ) ),
					[ "AA,00" ]			= (A / 2) * (1 - (-1) ^ ceil( idx / B ) ),
					[ "A,B" ]			= A + ((B - A) / 2) * (1 + (-1) ^ idx ),
					[ "AA,BB" ]			= A + ((B - A) / 2) * (1 + (-1) ^ ceil( idx / C ) ),
					[ "A,mB" ]			= B * ceil( idx / A ),
					[ "1-->A" ]			= A - A * ceil( idx / A ) + idx,
					[ "A-->1" ]			= A * ceil( idx / A ) - idx + 1,
					[ "1-->A,-1-->-A" ]	= (A - A * ceil( idx / A ) + idx) * (-1) ^ (ceil( idx / A ) + 1),
					[ "1-->A-->1" ]		= (A - 1 - (A - 1) * ceil( idx / (A - 1) ) + idx) * (-1) ^ (ceil( idx / (A - 1) ) + 1) + (A + 1) * (1 + (-1) ^ ceil( idx / (A - 1) )) / 2,
					[ "A-->1-->A" ]		= A + 1 - ((A - 1 - (A - 1) * ceil( idx / (A - 1) ) + idx) * (-1) ^ (ceil( idx / (A - 1) ) + 1) + (A + 1) * (1 + (-1) ^ ceil( idx / (A - 1) )) / 2),
					[ "A-->B-->A" ]		= (( A > B )
										and B + D - ((D - 1 - (D - 1) * ceil( idx / (D - 1) ) + idx) * (-1) ^ (ceil( idx / (D - 1) ) + 1) + (D + 1) * (1 + (-1) ^ ceil( idx / (D - 1) )) / 2)
										or  A + (E - 1 - (E - 1) * ceil( idx / (E - 1) ) + idx) * (-1) ^ (ceil( idx / (E - 1) ) + 1) + (E + 1) * (1 + (-1) ^ ceil( idx / (E - 1) )) / 2 - 1
										),
					[ "0,11" ]			= 1 - floor( 1 / idx ),
					[ "1,00" ]			= floor( 1 / idx ),
					[ "0,AA" ]			= A * (1 - floor( 1 / idx )),
					[ "A,00" ]			= A * floor( 1 / idx ),
					[ "A,BB" ]			= A * floor( 1 / idx ) + B * (1 - floor( 1 / idx )),		--math.i( j, 5, 7 )[ "A,BB" ]
					[ "0<->11" ]		= 1 - floor( (A * ceil( idx / A ) - idx + 1) / A ),			--math.i( j, 5 )[ "0<->11" ]
					[ "00<->1" ]		= floor( (A - A * ceil( idx / A ) + idx) / A ),				--math.i( j, 5 )[ "00<->1" ]
					[ "1<->00" ]		= floor( (A * ceil( idx / A ) - idx + 1) / A ),				--math.i( j, 5 )[ "1<->00" ]
					[ "11<->0" ]		= 1 - floor( (A - A * ceil( idx / A ) + idx) / A ),			--math.i( j, 5 )[ "11<->0" ]
					[ "0<->AA" ]		= A * (1 - floor( (B * ceil( idx / B ) - idx + 1) / B )),	--math.i( j, 5, 7 )[ "0<->AA" ]
					[ "00<->A" ]		= A * floor( (B - B * ceil( idx / B ) + idx) / B ),			--math.i( j, 5, 7 )[ "00<->A" ]
					[ "A<->00" ]		= A * floor( (B * ceil( idx / B ) - idx + 1) / B ),			--math.i( j, 5, 7 )[ "A<->00" ]
					[ "AA<->0" ]		= A * (1 - floor( (B - B * ceil( idx / B ) + idx) / B )),	--math.i( j, 5, 7 )[ "AA<->0" ]
					[ "A<->BB" ]		= A * floor( (C * ceil( idx / C ) - idx + 1) / C ) + B * (1 - floor( (C * ceil( idx / C ) - idx + 1) / C )),--math.i( j, 5, 2, 7 )[ "A<->BB" ]
					[ "AA<->B" ]		= A * (1 - floor( (C - C * ceil( idx / C ) + idx) / C )) + B * floor( (C - C * ceil( idx / C ) + idx) / C ),--math.i( j, 5, 2, 7 )[ "AA<->B" ]
					[ "N,n" ]			= floor( (idx - 1) / A ) + 1,
					--[ "Ac,Bd" ]			= A + ((B - A) / 2) * (1 + (-1) ^ ceil( idx / (C + ((F - C) / 2) * (1 + (-1) ^ idx )) ) ), --!_G.ke4.math.i( j, 9, 17, 3, 5 )[ "Ac,Bd" ]!
					--[ "Ac,Bd" ]			= ceil( idx / (C + ((F - C) / 2) * (1 + (-1) ^ idx )) ), --!_G.ke4.math.i( j, 9, 17, 3, 5 )[ "Ac,Bd" ]!
				}
				for k, val in pairs( algorithms ) do
					if tostring( val ) == "-0" then
						algorithms[ k ] = 0
					end
				end
				return algorithms
			end,
			
			circle = function( Shape )
				--retorna las coordenadas del centro y el radio de un círculo a partir de tres puntos en un clip
				local coor, center = { }, { }
				for c in Shape:gmatch( "%-?%d+[%.%d]*" ) do
					table.insert( coor, tonumber( c ) )
				end
				local P1 = { x = coor[ 1 ], y = coor[ 2 ], z = -(coor[ 1 ] ^ 2 + coor[ 2 ] ^ 2) }
				local P2 = { x = coor[ 3 ], y = coor[ 4 ], z = -(coor[ 3 ] ^ 2 + coor[ 4 ] ^ 2) }
				local P3 = { x = coor[ 5 ], y = coor[ 6 ], z = -(coor[ 5 ] ^ 2 + coor[ 6 ] ^ 2) }
				local Det_i = (P1.x * P2.y + P2.x * P3.y + P3.x * P1.y) - (P1.y * P2.x + P2.y * P3.x + P3.y * P1.x)
				local Det_D = (P1.z * P2.y + P2.z * P3.y + P3.z * P1.y) - (P1.y * P2.z + P2.y * P3.z + P3.y * P1.z)
				local Det_E = (P1.x * P2.z + P2.x * P3.z + P3.x * P1.z) - (P1.z * P2.x + P2.z * P3.x + P3.z * P1.x)
				local Det_F = (P1.x * P2.y * P3.z + P2.x * P3.y * P1.z + P3.x * P1.y * P2.z) - (P1.z * P2.y * P3.x + P2.z * P3.y * P1.x + P3.z * P1.y * P2.x)
				local Cd = Det_D / Det_i
				local Ce = Det_E / Det_i
				local Cf = Det_F / Det_i
				center.x = ke4.math.round( -Cd / 2, 3 )
				center.y = ke4.math.round( -Ce / 2, 3 )
				local radius = ke4.math.round( ((Cd / 2) ^ 2 + (Ce / 2) ^ 2 - Cf) ^ 0.5, 3 )
				return center.x, center.y, radius
			end,
			
			rotate = function( p, axis, angle )
				--rota un punto respecto a cualquiera de los 3 ejes
				local angle = rad( angle or 0 )
				local axisr = axis or "z"
				local rot_p = { }
				local coo_x = p[ 1 ] or 0
				local coo_y = p[ 2 ] or 0
				local coo_z = p[ 3 ] or 0
				if axisr == "x" then
					rot_p[ 1 ] = ke4.math.round( coo_x, 3 )
					rot_p[ 2 ] = ke4.math.round( cos( angle ) * coo_y - sin( angle ) * coo_z, 3 )
					rot_p[ 3 ] = ke4.math.round( sin( angle ) * coo_y + cos( angle ) * coo_z, 3 )
				elseif axisr == "y" then
					rot_p[ 1 ] = ke4.math.round( cos( angle ) * coo_x + sin( angle ) * coo_z, 3 )
					rot_p[ 2 ] = ke4.math.round( coo_y, 3 )
					rot_p[ 3 ] = ke4.math.round( cos( angle ) * coo_z - sin( angle ) * coo_x, 3 )
				elseif axisr == "z" then
					rot_p[ 1 ] = ke4.math.round( cos( angle ) * coo_x - sin( angle ) * coo_y, 3 )
					rot_p[ 2 ] = ke4.math.round( sin( angle ) * coo_x + cos( angle ) * coo_y, 3 )
					rot_p[ 3 ] = ke4.math.round( coo_z, 3 )
				end
				return rot_p
			end,
		
			trim = function( x, min, max )
				-- Trim number in range
				if type( x ) ~= "number" or type( min ) ~= "number" or type( max ) ~= "number" then
					error( "3 numbers expected", 2 )
				end
				return x < min and min or x > max and max or x
			end,
			
			matrix_sum = function( A, B )
				local sum
				if type( A ) == "table"
					and type( B ) == "table" then
					for _, v in ipairs( A ) do
						if type( v ) ~= "number" then
							error( "<<Error: ke4.math.matrix_sum>> La primera matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
					for _, v in ipairs( B ) do
						if type( v ) ~= "number" then
							error( "<<Error: ke4.math.matrix_sum>> La segunda matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
					sum = { }
					for i = 1, #A do
						sum[ i ] = A[ i ] + B[ i ]
					end
				elseif  type( A ) == "number"
					and type( B ) == "table" then
					for _, v in ipairs( B ) do
						if type( v ) ~= "number" then
							error( "<<Error: ke4.math.matrix_sum>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
					sum = { }
					for i = 1, #B do
						sum[ i ] = A + B[ i ]
					end
				elseif  type( A ) == "table"
					and type( B ) == "number" then
					for _, v in ipairs( A ) do
						if type( v ) ~= "number" then
							error( "<<Error: ke4.math.matrix_sum>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
					sum = { }
					for i = 1, #A do
						sum[ i ] = A[ i ] + B
					end
				else
					sum = A + B
				end
				return sum
			end,
			
			matrix_trans = function( A )
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_trans>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local trans = { }
				local n = (#A) ^ 0.5
				if n == ke4.math.round( n ) then
					for i = 1, #A do
						trans[ i ] = A[ n * ((i - 1) % n) + ceil( i / n ) ]
					end
					return trans
				end
				return A
			end,

			matrix_esc = function( A, Escalar )
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_esc>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local esc = { }
				for i = 1, #A do
					esc[ i ] = Escalar * A[ i ]
				end
				return esc
			end,
			
			matrix_mul = function( A, B )
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_mul>> La primera matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				for _, v in ipairs( B ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_mul>> La segunda matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local mul, trans = { }, { }
				local An, Bn = #A, #B
				if An < Bn then
					trans = ke4.math.matrix_trans( B )
					for i = 1, An do
						mul[ i ] = 0
						for k = 1, An do
							mul[ i ] = mul[ i ] + A[ k ] * trans[ An * ((i - 1) % An) + k ]
						end
					end
				elseif An > Bn then
					for i = 1, Bn do
						mul[ i ] = 0
						for k = 1, Bn do
							mul[ i ] = mul[ i ] + B[ i ] * A[ Bn * ((i - 1) % Bn) + k ]
						end
					end
				elseif An == Bn then
					local n = An ^ 0.5
					for i = 1, An do
						mul[ i ] = 0
						for k = 1, n do
							mul[ i ] = mul[ i ] + A[ (i - 1) % n + (k - 1) * n + 1 ] * B[ floor( (i - 1) / n ) * n + k ]
						end
					end
				end
				return mul
			end,
			
			matrix_mul2 = function( ... )
				local Matrixes = { ... }
				for i = 1, #Matrixes do
					if #Matrixes[ i ] ~= 9 then
						error( "<<Error: ke4.math.matrix_mul2>> Cada matriz debe ser de tamaño 3x3\n3x3 matrix expected", 2 )
					end
					for _, v in ipairs( Matrixes[ i ] ) do
						if type( v ) ~= "number" then
							error( "<<Error: ke4.math.matrix_mul2>> Cada matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
				end
				local Mul = {
					1,	0,	0,
					0,	1,	0,
					0,	0,	1
				}
				for i = 1, #Matrixes do
					Mul = ke4.math.matrix_mul( Mul, Matrixes[ i ] )
				end
				return Mul
			end,
			
			matrix_cof = function( A, Return )
				if #A ~= 9 then
					error( "<<Error: ke4.math.matrix_cof>> Debe ser una matriz 3x3\n3x3 matrix expected", 2 )
				end
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_cof>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local Ext = ke4.table.duplicate( A )
				for i = 1, 6 do
					table.insert( Ext, i )
				end
				for i = 1, 5 do
					table.insert( Ext, 5 * i - 1, Ext[ 5 * i - 3 ] )
					table.insert( Ext, 5 * i - 1, Ext[ 5 * i - 4 ] )
				end
				local cof = { }
				local n
				for i = 1, 9 do
					n = i + 2 * (ceil( i / 3 ) - 1)
					cof[ i ] = Ext[ n + 6 ] * Ext[ n + 12 ] - Ext[ n + 7 ] * Ext[ n + 11 ]
				end
				local det = 0
				for i = 1, 3 do
					det = det + A[ i ] * cof[ i ]
				end
				local adj = ke4.math.matrix_trans( cof )
				local inv = ke4.table.duplicate( A )
				if det ~= 0 then
					inv = ke4.math.matrix_esc( adj, 1 / det )
				end
				if Return == "det" then
					return det
				elseif Return == "adj" then
					return adj
				elseif Return == "inv" then
					if det ~= 0 then
						return inv
					end
					return nil
				end
				return cof
			end,
			
			matrix_det = function( A )
				if #A ~= 9 then
					error( "<<Error: ke4.math.matrix_det>> Debe ser una matriz 3x3\n3x3 matrix expected", 2 )
				end
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_det>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local det = ke4.math.matrix_cof( A, "det" )
				return det
			end,

			matrix_adj = function( A )
				if #A ~= 9 then
					error( "<<Error: ke4.math.matrix_adj>> Debe ser una matriz 3x3\n3x3 matrix expected", 2 )
				end
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_adj>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local adj = ke4.math.matrix_cof( A, "adj" )
				return adj
			end,

			matrix_inv = function( A )
				if #A ~= 9 then
					error( "<<Error: ke4.math.matrix_inv>> Debe ser una matriz 3x3\n3x3 matrix expected", 2 )
				end
				for _, v in ipairs( A ) do
					if type( v ) ~= "number" then
						error( "<<Error: ke4.math.matrix_inv>> La matriz solo debe contener números\nmatrix must contain only numbers", 2 )
					end
				end
				local inv = ke4.math.matrix_cof( A, "inv" )
				return inv
			end,
			
			matrix_dis = function( Disx, Disy ) --> displace
				local dis_x = Disx or 0
				local dis_y = Disy or 0
				local Dis = {
					1,		0,		0,
					0,		1,		0,
					dis_x,	dis_y,	1
				}
				return Dis
			end,
			
			matrix_rot = function( Angle, Axis, Orgx, Orgy ) --> rotation
				local AngR = -rad( Angle or 0 )
				local Orgx = Orgx or 0
				local Orgy = Orgy or 0
				local Rot_x = {
					1,	  0,			0,
					0,	  cos( AngR ),	sin( AngR ),
					0,	  -sin( AngR ), cos( AngR )
				}
				local Rot_y = {
					cos( AngR ),  0,	sin( AngR ),
					0,			  1,	0,
					-sin( AngR ), 0,	cos( AngR )
				}
				local Rot_z = {
					cos( AngR ),  sin( AngR ),	0,
					-sin( AngR ), cos( AngR ),	0,
					0,			  0,			1
				}
				local Rot = Rot_z
				if Axis == "x" then
					Rot = Rot_x
				elseif Axis == "y" then
					Rot = Rot_y
				end
				local Rot1 = ke4.math.matrix_mul( ke4.math.matrix_dis( Orgx, Orgy ), Rot )
				return ke4.math.matrix_mul( Rot1, ke4.math.matrix_dis( -Orgx, -Orgy ) )
			end,
			
			matrix_rat = function( Ratx, Raty, Orgx, Orgy ) --> ratio
				local esc_x = Ratx or 1
				local esc_y = Raty or esc_x
				local org_x = Orgx or 0
				local org_y = Orgy or 0
				local Rat = {
					esc_x,	0,		0,
					0,		esc_y,	0,
					0,		0,		1
				}
				local Rat1 = ke4.math.matrix_mul( ke4.math.matrix_dis( org_x, org_y ), Rat )
				return ke4.math.matrix_mul( Rat1, ke4.math.matrix_dis( -org_x, -org_y ) )
			end,
			
			matrix_ref = function( Mode ) --> reflection
				local Ref_x = {
					1,	0,	0,
					0,	-1,	0,
					0,	0,	1
				}
				local Ref_y = {
					-1,	0,	0,
					0,	1,	0,
					0,	0,	1
				}
				local Ref_org = {
					-1,	0,	0,
					0,	-1,	0,
					0,	0,	1
				}
				local Ref_yequx = {
					0,	1,	0,
					1,	0,	0,
					0,	0,	1
				}
				local Ref = Ref_yequx
				if Mode == "x" then
					Ref = Ref_x
				elseif Mode == "y" then
					Ref = Ref_y
				elseif Mode == "o" then
					Ref = Ref_org
				end
				return Ref
			end,

			matrix_fil = function( Filxy, Axis ) --> oblique
				if type( Filxy ) == "function" then
					Filxy = Filxy( )
				end
				local fil_px, fil_py = 0, 0
				if type( Filxy ) == "number" then
					fil_px, fil_py = Filxy, Filxy
				elseif type( Filxy ) == "table" then
					fil_px, fil_py = Filxy[ 1 ], Filxy[ 2 ]
				end
				local Fil_x = {
					1,		 0,	0,
					-fil_px, 1,	0,
					0,		 0,	1
				}
				local Fil_y = {
					1,	fil_py,	0,
					0,	1,		0,
					0,	0,		1
				}
				local Fil = Fil_x
				if Axis == "y" then
					Fil = Fil_y
				end
				if type( Filxy ) == "table" then
					return ke4.math.matrix_mul( Fil_x, Fil_y )
				end
				return Fil
			end,
			
			bezier = function( j, maxj, Return, ... )
				if type( ... ) ~= "string" then
					local points_Bezier = { ... }
					if type( ... ) == "table" then
						points_Bezier = ...
					end
					local MB_points_Bx, MB_points_By = { }, { }
					for i = 1, #points_Bezier / 2 do
						MB_points_Bx[ i ] = points_Bezier[ 2 * i - 1 ]
						MB_points_By[ i ] = points_Bezier[ 2 * i - 0 ]
					end
					local MBpos_x, MBpos_y = ke4.math.confi_bezier( nil, MB_points_Bx, MB_points_By, (j - 1) / (maxj - 1) )
					if Return == "x" then
						return ke4.math.round( MBpos_x, 3 )
					elseif Return == "y" then
						return ke4.math.round( MBpos_y, 3 )
					end
					return ke4.math.round( MBpos_x, 3 ), ke4.math.round( MBpos_y, 3 )
				else
					--if j == 1 then
						local Shape9 = ke4.shape.ASSDraw3( ... )
						local max_space = 1
						local MB_t, MB_i, MB_points_Bezier = 0, 1, { }
						MBpos_x, MBpos_y, MB_m = { }, { }, 0
						for sh_c in Shape9:gmatch( "%S+" ) do
							table.insert( MB_points_Bezier, sh_c )
						end
						local MB_lpos_x, MB_lpos_y = MB_points_Bezier[ 2 ], MB_points_Bezier[ 3 ]
						while MB_i <= #MB_points_Bezier do
							MB_points_Bx, MB_points_By = { }, { }
							if MB_points_Bezier[ MB_i ] == "m" then
								s_point_x = MB_points_Bezier[ MB_i + 1 ]
								s_point_y = MB_points_Bezier[ MB_i + 2 ]
								MB_i = MB_i + 3
								MB_ldist = 0
							elseif MB_points_Bezier[ MB_i ] == "b" then
								MB_points_Bx[ 1 ] = s_point_x
								MB_points_By[ 1 ] = s_point_y
								for k = 2, 4 do
									MB_points_Bx[ k ] = MB_points_Bezier[ MB_i + 2 * (k - 2) + 1 ]
									MB_points_By[ k ] = MB_points_Bezier[ MB_i + 2 * (k - 2) + 2 ]
								end
								s_point_x = MB_points_Bezier[ MB_i + 5 ]
								s_point_y = MB_points_Bezier[ MB_i + 6 ]
								MB_i = MB_i + 7
							elseif MB_points_Bezier[ MB_i ] == "l" then
								MB_points_Bx[ 1 ] = s_point_x
								MB_points_By[ 1 ] = s_point_y
								MB_points_Bx[ 2 ] = MB_points_Bezier[ MB_i + 1 ]
								MB_points_By[ 2 ] = MB_points_Bezier[ MB_i + 2 ]
								s_point_x = MB_points_Bezier[ MB_i + 1 ]
								s_point_y = MB_points_Bezier[ MB_i + 2 ]
								MB_i = MB_i + 3
							else
								MB_i = #MB_points_Bezier + 1
							end
							MB_ct = 0
							MB_n = #MB_points_Bx
							if MB_n ~= 0 then
								while MB_ct >= 0
									and MB_ct < 1 do
									MB_m = MB_m + 1
									if MB_ct == 0 then
										if MB_points_Bx[ 1 ] == MB_lpos_x
											and MB_points_By[ 1 ] == MB_lpos_y then
											MBpos_x[ MB_m ], MBpos_y[ MB_m ] = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, 0.12 )
											MB_dist = ke4.math.distance( MBpos_x[ MB_m ], MBpos_y[ MB_m ], MB_points_Bx[ 1 ], MB_points_By[ 1 ] )
											if MB_dist == 0 then
												MB_ct = 1
											else
												MB_ct = (max_space - MB_ldist) / MB_dist * 0.1
												MBpos_x[ MB_m ], MBpos_y[ MB_m ] = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, MB_ct )
												MB_nx, MB_ny = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, MB_ct + 0.1 )
												MB_dist = ke4.math.distance( MB_nx, MB_ny, MBpos_x[ MB_m ], MBpos_y[ MB_m ] )
												MB_ct = MB_ct + max_space / MB_dist * 0.1
											end
										else
											MBpos_x[ MB_m ], MBpos_y[ MB_m ] = MB_points_Bx[ 1 ], MB_points_By[ 1 ]
											MB_nx, MB_ny = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, MB_ct + 0.1 )
											MB_dist = ke4.math.distance( MB_nx, MB_ny, MBpos_x[ MB_m ], MBpos_y[ MB_m ] )
											if MB_dist == 0 then
												MB_ct = 1
											else
												MB_ct = MB_ct + max_space / MB_dist * 0.1
											end
										end
									else
										MBpos_x[ MB_m ], MBpos_y[ MB_m ] = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, MB_ct )
										MB_nx, MB_ny = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, MB_ct + 0.1 )
										MB_dist = ke4.math.distance( MB_nx, MB_ny, MBpos_x[ MB_m ], MBpos_y[ MB_m ] )
										if MB_dist == 0 then
											MB_ct = 1
										else
											MB_ct = MB_ct + max_space / MB_dist * 0.1
										end
									end
								end
								MB_lpos_x, MB_lpos_y = ke4.math.confi_bezier( MB_n, MB_points_Bx, MB_points_By, 1 )
								MB_ldist = ke4.math.distance( MB_lpos_x, MB_lpos_y, MBpos_x[ MB_m ], MBpos_y[ MB_m ] )
								if MB_ldist > max_space then
									MB_ldist = max_space
								end
							end
						end
					--end
					if Return == "x" then
						return ke4.math.round( MBpos_x[ j ], 3 )
					elseif Return == "y" then
						return ke4.math.round( MBpos_y[ j ], 3 )
					end
					return ke4.math.round( MBpos_x[ j ], 3 ), ke4.math.round( MBpos_y[ j ], 3 )
				end
			end,

			bezier2 = function( pct, pts )
				-- Get point on n-degree bezier curve
				if type( pct ) ~= "number" or pct < 0 or pct > 1 or type( pts ) ~= "table" then
					error( "percent number and points table expected", 2 )
				end
				local pts_n = #pts
				if pts_n < 2 then
					error( "at least 2 points expected", 2 )
				end
				for _, value in ipairs(pts) do
					if type( value[ 1 ] ) ~= "number" or type( value[ 2 ] ) ~= "number" or (value[ 3 ] ~= nil and type( value[ 3 ] ) ~= "number") then
						error( "points have to be tables with 2 or 3 numbers", 2 )
					end
				end
				local pct_inv = 1 - pct
				if pts_n == 2 then
					return pct_inv * pts[ 1 ][ 1 ] + pct * pts[ 2 ][ 1 ], pct_inv * pts[ 1 ][ 2 ] + pct * pts[ 2 ][ 2 ],
					pts[ 1 ][ 3 ] and pts[ 2 ][ 3 ] and pct_inv * pts[ 1 ][ 3 ] + pct * pts[ 2 ][ 3 ] or 0
				elseif pts_n == 3 then
					return pct_inv * pct_inv * pts[ 1 ][ 1 ] + 2 * pct_inv * pct * pts[ 2 ][ 1 ] + pct * pct * pts[ 3 ][ 1 ],
					pct_inv * pct_inv * pts[ 1 ][ 2 ] + 2 * pct_inv * pct * pts[ 2 ][ 2 ] + pct * pct * pts[ 3 ][ 2 ],
					pts[ 1 ][ 3 ] and pts[ 2 ][ 3 ] and pct_inv * pct_inv * pts[ 1 ][ 3 ] + 2 * pct_inv * pct * pts[ 2 ][ 3 ] + pct * pct * pts[ 3 ][ 3 ] or 0
				elseif pts_n == 4 then
					return pct_inv * pct_inv * pct_inv * pts[ 1 ][ 1 ] + 3 * pct_inv * pct_inv * pct * pts[ 2 ][ 1 ] + 3 * pct_inv * pct * pct * pts[ 3 ][ 1 ] + pct * pct * pct * pts[ 4 ][ 1 ],
					pct_inv * pct_inv * pct_inv * pts[ 1 ][ 2 ] + 3 * pct_inv * pct_inv * pct * pts[ 2 ][ 2 ] + 3 * pct_inv * pct * pct * pts[ 3 ][ 2 ] + pct * pct * pct * pts[ 4 ][ 2 ],
					pts[ 1 ][ 3 ] and pts[ 2 ][ 3 ] and pts[ 3 ][ 3 ] and pts[ 4 ][ 3 ] and pct_inv * pct_inv * pct_inv * pts[ 1 ][ 3 ] + 3 * pct_inv * pct_inv * pct * pts[ 2 ][ 3 ] + 3 * pct_inv * pct * pct * pts[ 3 ][ 3 ] + pct * pct * pct * pts[ 4 ][ 3 ] or 0
				else
					local function fac( n )
						local k = 1
						for i = 2, n do
							k = k * i
						end
						return k
					end
					local ret_x, ret_y, ret_z = 0, 0, 0
					local n, bern, pt = pts_n - 1
					for i = 0, n do
						pt = pts[ 1 + i ]
						bern = fac( n ) / (fac( i ) * fac( n - i )) * pct ^ i * pct_inv ^ (n - i)
						ret_x = ret_x + pt[ 1 ] * bern
						ret_y = ret_y + pt[ 2 ] * bern
						ret_z = ret_z + (pt[ 3 ] or 0) * bern
					end
					return ret_x, ret_y, ret_z
				end
			end,
			
			to16 = function( Num )
				--decimal to hexadecimal
				local dec_to_hex = Num
				if type( Num ) == "number" then
					dec_to_hex = format( "%X", ke4.math.round( Num ) )
				elseif type( Num ) == "table" then
					dec_to_hex = { }
					for k, v in pairs( Num ) do
						if type( v ) == "number" then
							dec_to_hex[ k ] = format( "%X", ke4.math.round( v ) )
						else
							dec_to_hex[ k ] = v
						end
					end
				end
				return dec_to_hex
			end, --!_G.ke4.math.to16( 255 )!

			clamp = function( Num, Min, Max )
				--restringe un número entre un mínimo y un máximo
				local Min = Min or 0
				local Max = Max or 1
				local c_min = math.min( Min, Max )
				local c_max = math.max( Min, Max )
				if type( Num ) == "number" then
					if Num < c_min then
						return c_min
					elseif Num > c_max then
						return c_max
					end
				end
				return Num
			end, --!_G.ke4.math.clamp( 3, 5, 10 )!
			
			clamp2 = function( Num, Min, Max )
				--restringe un número entre un mínimo y un máximo,
				--pero en caso de excederse, se empieza a devolver
				local Min = Min or 0
				local Max = Max or 1
				local Num = ke4.math.round( Num * 10000 )
				Min = ke4.math.round( Min * 10000 )
				Max = ke4.math.round( Max * 10000 )
				return ke4.math.round( ke4.math.i( Num - Min, Min, Max )[ "A-->B-->A" ] / 10000, 3 )
			end,  --!_G.ke4.math.clamp2( 3 * (j - 1) / (maxj - 1) )!
			
			audio = function( Audio, Time, Start, Loops, Scale, Offset_time, Values )
				--convierte un archivo .wav en valores o transformaciones para shapes :D
				local Wav_fil = Audio or "test.wav"
				local Wav_frm = 2 * frame_dur
				local Wav_max = Loops or 12
				local Wav_scl = Scale or 1 / 86 -- = 0.0116 vertical scale
				local Wav_otm = Offset_time or 0
				if Wav_scl <= 0 then
					Wav_scl = abs( Wav_scl )
				end
				if Wav_fil:match( "%.wav" ) == nil then
					Wav_fil = Wav_fil .. ".wav"
				end
				if Wav_max <= 0 then
					Wav_max = 2
				end
				local reader = ke4.decode.create_wav_reader( Wav_fil )
				local chunk_size = reader.sample_from_ms( Wav_frm )
				local max_transfos = ceil( (Time + 2 * Wav_otm) / Wav_frm )
				local barras = ceil( Wav_max )
				local vals_tbl, idx, ms
				local val_y_tbl = { }
				for i = 1, max_transfos do
					ms = Start + Wav_frm * (i - 1)
					reader.position( floor( reader.sample_from_ms( ms ) ) )
					vals_tbl = reader.samples( chunk_size )[ 1 ]
					idx = ceil( (#vals_tbl + 1) / (barras + 1) )
					val_y_tbl[ i ] = { }
					for k = 1, barras do
						val_y_tbl[ i ][ k ] = vals_tbl[ k * idx ]
					end
				end
				local transfos_vals = { }
				for i = 1, barras do
					transfos_vals[ i ] = { }
					for k = 1, max_transfos do
						transfos_vals[ i ][ k ] = floor( abs( val_y_tbl[ k ][ i ] ) * Wav_scl )
					end
				end
				local transfos_tags = { }
				for i = 1, barras do
					transfos_tags[ i ] = format( "\\fscy%s", transfos_vals[ i ][ 1 ] )
					for k = 2, max_transfos do
						transfos_tags[ i ] = transfos_tags[ i ] .. format( "\\t(%s,%s,\\fscy%s)", (k - 1) * Wav_frm, k * Wav_frm, transfos_vals[ i ][ k ] )
					end
				end
				if Values then
					return transfos_vals
				end
				return transfos_tags
				--code line: bars = _G.ke4.math.audio( "test.wav", line.duration, line.start_time, 20 )
				--!maxloop( #bars )!{\an2\pos(!$lleft + 30 * (j - 1)!,$bottom)\bord0\shad0\fscx25!bars[ j ]!\p1}!_G.ke4.shape.rectangle!
			end,

		},
		
		-- Random sublibrary
		random = {
			color = function( H, S, V )
				--retorna un color al azar o con parámetros especiíficos
				--los parámetros van de {0, 360}, {0, 100}, {0, 100}
				local Hrc, Src, Vrc = ke4.math.R( 360 ), 1, 1
				if type( H ) == "table" then
					Hrc = ke4.math.R( (H[ 1 ] - 1) % 360 + 1, (H[ 2 ] - 1) % 360 + 1 )
				elseif type( H ) == "number" then
					Hrc = (H - 1) % 360 + 1
				end
				if type( S ) == "table" then
					Src = ke4.math.R( S[ 2 ] % 101, S[ 1 ] % 101 ) / 100
				elseif type( S ) == "number" then
					Src = ke4.math.i( S + 1, 0, 100 )[ "A-->B-->A" ] / 100
				end
				if type( V ) == "table" then
					Vrc = ke4.math.R( V[ 2 ] % 101, V[ 1 ] % 101 ) / 100
				elseif type( V ) == "number" then
					Vrc = ke4.math.i( V + 1, 0, 100 )[ "A-->B-->A" ] / 100
				end
				return ke4.color.HSV_to_RGB( Hrc, Src, Vrc )
			end, --{\c!_G.ke4.random.color( )!}
			
			color2 = function( H, S, V )
				--retorna un color al azar o con parámetros especiíficos
				--los parámetros van de {0, 360}, {0, 1}, {0, 1}
				local Hrc, Src, Vrc = ke4.math.R( 360 ), 1, 1
				if type( H ) == "table" then
					Hrc = ke4.math.R( (H[ 1 ] - 1) % 360 + 1, (H[ 2 ] - 1) % 360 + 1 )
				elseif type( H ) == "number" then
					Hrc = (H - 1) % 360 + 1
				end
				if type( S ) == "table" then
					Src = ke4.math.R( (100 * S[ 2 ]) % 101, (100 * S[ 1 ]) % 101 ) / 100
				elseif type( S ) == "number" then
					Src = ke4.math.i( 100 * S + 1, 0, 100 )[ "A-->B-->A" ] / 100
				end
				if type( V ) == "table" then
					Vrc = ke4.math.R( (100 * V[ 2 ]) % 101, (100 * V[ 1 ]) % 101 ) / 100
				elseif type( V ) == "number" then
					Vrc = ke4.math.i( 100 * V + 1, 0, 100 )[ "A-->B-->A" ] / 100
				end
				return ke4.color.HSV_to_RGB( Hrc, Src, Vrc )
			end, --{\c!_G.ke4.random.color2( )!}

			colorvc = function( H, S, V )
				--retorna un color (VC) al azar o con parámetros especiíficos
				return format( "(%s,%s,%s,%s)",
					ke4.random.color( H, S, V ), ke4.random.color( H, S, V ),
					ke4.random.color( H, S, V ), ke4.random.color( H, S, V )
				)
			end,
			
			alpha = function( alpha_i, alpha_f )
				--retorna un valor alpha al azar o con parámetros especiíficos
				local ra_i, ra_f = 0, 255
				if type( alpha_i ) == "string" then
					ra_i = tonumber( alpha_i:match( "(%x%x)" ), 16 )
				elseif type( alpha_i ) == "number" then
					ra_i = ke4.math.i( alpha_i + 1, 0, 255 )[ "A-->B-->A" ]
				end
				if type( alpha_f ) == "string" then
					ra_f = tonumber( alpha_f:match( "(%x%x)" ), 16 )
				elseif type( alpha_f ) == "number" then
					ra_f = ke4.math.i( alpha_f + 1, 0, 255 )[ "A-->B-->A" ]
				end
				return ke4.alpha.val2ass( ke4.math.R( ra_f, ra_i ) )
			end,
			
			alphava = function( Ai, Af )
				--retorna un valor alpha (VA) al azar o con parámetros especiíficos
				return format( "(%s,%s,%s,%s)",
					ke4.random.alpha( Ai, Af ), ke4.random.alpha( Ai, Af ),
					ke4.random.alpha( Ai, Af ), ke4.random.alpha( Ai, Af )
				)
			end,

			e = function( ... )
				-- retorma un elemento al azar de un listado de elementos o de una tabla
				local Table_e = { ... }
				if type( ... ) == "table" then
					Table_e = ...
				end
				return Table_e[ ke4.math.R( #Table_e ) ]
			end,
			
			unique = function( table_or_number, index_r )
				--( table_or_number[, index_r] )
				local Table_u, Ind = recall.tableu, 1
				if index_r == nil then
					return ke4.table.disorder( table_or_number )
				end
				if index_r == 1 then
					Table_u = remember( "tableu", ke4.table.disorder( table_or_number ) )
				end
				Ind = #Table_u - #Table_u * ceil( index_r / #Table_u ) + index_r
				return Table_u[ Ind ]
			end,
		},

		-- Algorithm sublibrary
		algorithm = {
			frames = function( starts, ends, dur )
				-- Creates iterator through frame times
				if type( starts ) ~= "number" or type( ends ) ~= "number" or type( dur ) ~= "number" or dur == 0 then
					error( "start, end and duration number expected", 2 )
				end
				local i, n = 0, ceil( (ends - starts) / dur )
				return function( )
					i = i + 1
					if i <= n then
						local ret_starts = starts + (i - 1) * dur
						local ret_ends = ret_starts + dur
						if dur < 0 and ret_ends < ends or dur > 0 and ret_ends > ends then
							ret_ends = ends
						end
						return ret_starts, ret_ends, i, n
					end
				end
			end,
			
			lines = function( text )
				-- Creates iterator through text lines
				if type( text ) ~= "string" then
					error( "string expected", 2 )
				end
				return function( )
					if text then
						local cr = text:find( "\r", 1, true )
						local lf = text:find( "\n", 1, true )
						local text_end, next_step = #text, 0
						if lf then
							text_end, next_step = lf - 1, 2
						end
						if cr then
							if not lf or cr < lf - 1 then
								text_end, next_step = cr - 1, 2
							elseif cr == lf - 1 then
								text_end, next_step = cr - 1, 3
							end
						end
						local line = text:sub( 1, text_end )
						if next_step == 0 then
							text = nil
						else
							text = text:sub( text_end + next_step )
						end
						return line
					end
				end
			end
		},
		
		-- String complement sublibrary
		string = {
			count = function( String, Capture )
				-- cantidad de veces que una captura o familia de capturas aparecen en un string
				local String = String or "ke4.string.count"
				local Capture = Capture or "KE"
				local str_count = 0
				if type( Capture ) == "string" then
					if Capture == "number" then
						Capture = "%-?%d+[%.%d]*"
					elseif Capture == "color" then
						Capture = "[%&%#Hh]^*%x%x%x%x%x%x[%&]*"
					elseif Capture == "alpha" then
						Capture = "%&[Hh]^*%x%x%&"
					elseif Capture == "shape" then
						Capture = "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*"
					end
					for cap in String:gmatch( Capture ) do
						str_count = str_count + 1
					end
				elseif type( Capture ) == "table" then
					for i = 1, #Capture do
						if Capture[ i ] == "number" then
							Capture[ i ] = "%-?%d+[%.%d]*"
						elseif Capture[ i ] == "color" then
							Capture[ i ] = "[%&%#Hh]^*%x%x%x%x%x%x[%&]*"
						elseif Capture[ i ] == "alpha" then
							Capture[ i ] = "%&[Hh]^*%x%x%&"
						elseif Capture[ i ] == "shape" then
							Capture[ i ] = "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*"
						end
						if type( Capture[ i ] ) == "string" then
							for cap in String:gmatch( Capture[ i ] ) do
								str_count = str_count + 1
							end
						end
					end
				end
				return str_count
			end, --!_G.ke4.string.count( "&HF58628&", "%x" )!
			

			toval = function( String )
				--convierte un string en el valor real que representa
				local String = String or ""
				local str_to_val
				--local line = linefx[ ii ]
				if type( String ) == "string" then
					String = String:gsub( "%.line", ".LINE" )
					String = String:gsub( "meta%.res_x", "xres" ):gsub( "meta%.res_y", "yres" )
					:gsub( "line%.i", "l_counter" ):gsub( "line%.n", "maxil_counter" )
					:gsub( "line%.", "linefx[  ii  ]." )
					:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
					:gsub( "%.LINE", ".line" )
					if pcall( loadstring( format( "return function( meta, line, x, y ) return %s end", String ) ) ) then
						local string_to_funct = loadstring( format( "return function( meta, line, x, y ) return %s end", String ) )( )
						if pcall( string_to_funct ) then
							str_to_val = string_to_funct( meta, line, x, y )
							if str_to_val then
								return str_to_val
							end
							String = String:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
							:gsub( "l_counter", "line.i" ):gsub( "maxil_counter", "line.n" )
							:gsub( "linefx%[  ii  %]%.", "line." )
							:gsub( "\"(&H%x+&)\"", "%1" )--> deja al string tal cual
							return String
						end --string.toval( "line.nil + 2" )
					end
					String = String:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
					:gsub( "l_counter", "line.i" ):gsub( "maxil_counter", "line.n" )
					:gsub( "linefx%[  ii  %]%.", "line." )
					:gsub( "\"(&H%x+&)\"", "%1" )--> deja al string tal cual
					return String
				end
				return String
			end, --!_G.ke4.string.toval( "5 + 7" )!

			i = function( String )
				-- convierte al string "i" en un valor numérico consecutivo
				local count_i = 0
				local String = String or ""
				String = String:gsub( "\\%w+%b()",
					function( capture )
						local capture_i = capture
						local capture = capture:gsub( "([%+%-%*%/%^%(%[%{%d%% ]^*)(i)([%+%-%*%/%^%)%]%}%%%\\ ]^*)",
							function( capture_ini, capture_i, capture_fin )
								local cap_ini, cap_i, cap_fin = capture_ini, count_i, capture_fin
								if capture_ini:match( "%d" ) then
									return cap_ini .. " * " .. cap_i .. cap_fin
								end
								return cap_ini .. cap_i .. cap_fin
							end
						)
						if capture_i ~= capture then
							count_i = count_i + 1
						end
						return capture
					end
				)
				return String
			end, --!_G.ke4.string.i( "\\fr( -5i )\\frx( 10 - i )" )!
			
			change = function( String, Capture, NoChange, NoCapture, Change )
				-- elimina o cambia una captura específica de un string
				local String = String or ""
				local Change = Change or ""
				local nocapture_tbl = { }
				if NoCapture then
					if type( NoCapture ) == "table" then
						for i = 1, #NoCapture do
							for nocap in String:gmatch( NoCapture[ i ] ) do
								table.insert( nocapture_tbl, nocap )
							end
							String = String:gsub( NoCapture[ i ], "<nocap>" )
						end
					else
						for nocap in String:gmatch( NoCapture ) do
							table.insert( nocapture_tbl, nocap )
						end
						String = String:gsub( NoCapture, "<nocap>" )
					end
				end
				if NoChange then
					if type( NoChange ) == "number" then
						NoChange = { NoChange }
					end
				else
					NoChange = { 0 }
				end
				local del_count
				if type( Capture ) == "string" then
					del_count = 0
					String = String:gsub( Capture,
						function( capture )
							del_count = del_count + 1
							if ke4.table.inside( NoChange, del_count ) then
								return capture
							end
							if type( Change ) == "function" then
								return Change( capture )
							end
							return Change
						end
					)
				else
					for i = 1, #Capture do
						del_count = 0
						String = String:gsub( Capture[ i ],
							function( capture )
								del_count = del_count + 1
								if ke4.table.inside( NoChange, del_count ) then
									return capture
								end
								if type( Change ) == "function" then
									return Change( capture )
								end
								return Change
							end
						)
					end
				end
				if NoCapture then
					nocap_count = 0
					String = String:gsub( "<nocap>",
						function( nocap_capture )
							nocap_count = nocap_count + 1
							return nocapture_tbl[ nocap_count ]
						end
					)
				end
				return String
			end, --!_G.ke4.string.change( "\\c&H0000FF&\\t(\\c&HFF00AA&)\\c&H00FFFF&ru", "\\c&H%x+&", 1, "\\t%b()" )!
			
			cap = function( String, Capture, Extra_Capture, Filter )
				-- ampliación de string.sub y string.gsub con la opción de capturas extras
				local String = String or ""
				local Filter = Filter or ""
				if type( Capture ) == "number"
					and type( Extra_Capture ) == "number" then
					local char_i, char_f = ceil( Capture ), ceil( Extra_Capture )
					-- captura un tramo del String desde char_i hasta char_f, como String:sub( )
					-- si char_i es mayor que char_f, entonces captura en tramo de forma inversa
					local chars_tbl, chars_sub = { }, ""
					for c in unicode.chars( String ) do
						table.insert( chars_tbl, c )
					end
					local chars_n = #chars_tbl
					--------------------------------
					if char_i < 0 then
						if char_i < -chars_n then
							char_i = -chars_n
						end
						char_i = chars_n + char_i + 1
					end
					if char_i > chars_n then
						char_i = chars_n
					end
					--------------------------------
					if char_f < 0 then
						if char_f < -chars_n then
							char_f = -chars_n
						end
						char_f = chars_n + char_f + 1
					end
					if char_f > chars_n then
						char_f = chars_n
					end
					--------------------------------
					local Steep = 1
					if type( Filter ) == "number" then
						-- string.cap( line.text_stripped, 4, 10, 0 )
						-- retorna la parte "negativa" de la captura
						local char_0i, char_0f = math.min( char_i, char_f ), math.max( char_i, char_f )
						for i = 1, chars_n do
							if i < char_0i
								or i > char_0f then
								chars_sub = chars_sub .. chars_tbl[ i ]
							end
						end
					else
						if char_i > char_f then
							Steep = -1
						end
						for i = char_i, char_f, Steep do
							chars_sub = chars_sub .. chars_tbl[ i ]
						end
					end
					String = chars_sub
				elseif type( Capture ) == "string"
					and type( Extra_Capture ) == "table" then
					local Captures_tbl = { }
					if #Extra_Capture > 0 then
						for i = 1, #Extra_Capture do
							Captures_tbl[ i ] = Capture
							for k = 1, #Extra_Capture - i + 1 do
								Captures_tbl[ i ] = Captures_tbl[ i ] .. Extra_Capture[ k ]
							end
						end
					end
					Captures_tbl[ #Captures_tbl + 1 ] = Capture
					for i = 1, #Captures_tbl do
						String = String:gsub( Captures_tbl[ i ], Filter )
					end
				end
				return String
				--str = "de45yo34 de34g de34g7 de de67g"
				--string.cap( str, "(de)", { "(%d+)", "(g)" }, function( A, B, C ) if C then return ";3" elseif B then return ";2" end return ";1" end )
			end,

			coupling = function( String, Modified, Group )
				local cap_exp = {
					[ 1 ] = {
						[ 1 ] = "(%%-?[%%d&]^*[%%.%%d&H%%x]*)",
						[ 2 ] = "(%%-?[%%d&]^*[%%.%%d&H%%x]*)"
					},
					[ 2 ] = {
						[ 1 ] = "(%%b())",
						[ 2 ] = "(%%-?[%%d&]^*[%%.%%d&H%%x]*)"
					},
					[ 3 ] = {
						[ 1 ] = "(%%-?[%%d&]^*[%%.%%d&H%%x]*)",
						[ 2 ] = "(%%b())"
					}
				}
				local cap_rep = {
					[ 1 ] = "%1(%2)%3(%4)",
					[ 2 ] = "%1%2%3(%4)",
					[ 3 ] = "%1(%2)%3%4"
				}
				local String = String or ""
				if Group then
					cap_rep = {
						[ 1 ] = "%1" .. Group:sub( 1, 1 ) .. "%2" .. Group:sub( 2, 2 ) .. "%3" .. Group:sub( 1, 1 ) .. "%4" .. Group:sub( 2, 2 ),
						[ 2 ] = "%1%2%3" .. Group:sub( 1, 1 ) .. "%4" .. Group:sub( 2, 2 ),
						[ 3 ] = "%1" .. Group:sub( 1, 1 ) .. "%2" .. Group:sub( 2, 2 ) .. "%3%4"
					}
				end
				local capx
				for i = 1, 3 do
					capx = Modified:gsub( "(%b())(%b())(%b())(%b())", "%1" .. cap_exp[ i ][ 1 ] .. "%3" .. cap_exp[ i ][ 2 ] )
					String = String:gsub( capx, cap_rep[ i ] )
				end
				return String
			end, --!_G.ke4.string.coupling( "tag8<>9", "(tag)(__)(<>)(__)", "[]" )!
			
			parts = function( String, Parts )
				local String = String or "effector-auto4.lua"
				local Parts = Parts or 3
				if type( Parts ) == "number" then
					Parts = ceil( abs( Parts ) )
				end
				if type( Parts ) == "table" then
					if Parts[ 1 ] <= 0 then
						Parts[ 1 ] = 1
					end
					if Parts[ 2 ] <= 0 then
						Parts[ 2 ] = 2
					end
				end
				if Parts == 0 then
					Parts = 2
				end
				local Parts_tbl, Chars_tbl = { }, { }
				for c in unicode.chars( String ) do
					table.insert( Chars_tbl, c )
				end
				local Part_i, i = 0, 1
				while #Chars_tbl > 0 do
					Part_i = Parts
					if type( Parts ) == "table" then
						Part_i = ke4.math.R( Parts[ 1 ], Parts[ 2 ] )
					end
					Parts_tbl[ i ] = ""
					for k = 1, Part_i do
						Parts_tbl[ i ] = Parts_tbl[ i ] .. (Chars_tbl[ 1 ] or "")
						table.remove( Chars_tbl, 1 )
					end
					i = i + 1
				end
				return Parts_tbl
				--retorna una tabla con las partes de n tamaño de un string
			end --!_G.ke4.table.view( _G.ke4.string.parts( "ejemplo de string", 2 ) )!
		},
		
		-- Text sublibrary
		text = {
			to_shape = function( Text_Config, Text, Scale, without )
				local Text = Text or "text.to_shape"
				while Text:sub( -1, -1 ) == " " do
					Text = Text:sub( 1, -2 )
				end
				local text_scale = Scale or 1
				if Text_Config.styleref then
					Text_Config = Text_Config.styleref
				end
				local Text_Confix = {
					[ 1 ] = Text_Config.fontname,
					[ 2 ] = Text_Config.bold,
					[ 3 ] = Text_Config.italic,
					[ 4 ] = Text_Config.underline,
					[ 5 ] = Text_Config.strikeout,
					[ 6 ] = Text_Config.fontsize,
					[ 7 ] = Text_Config.scale_x * text_scale / 100,
					[ 8 ] = Text_Config.scale_y * text_scale / 100,
					[ 9 ] = Text_Config.spacing
				}
				local text_font = ke4.decode.create_font( unpack( Text_Confix ) )
				local text_shape = ke4.shape.ASSDraw3( text_font.text_to_shape( Text ) )
				if without then
					return text_shape
				end
				local width, height = aegisub.text_extents( Text_Config, Text )
				local text_off_x = 0.5 * (ke4.shape.width( text_shape ) - text_scale * width)
				local text_off_y = 0.5 * (ke4.shape.height( text_shape ) - text_scale * height)
				text_shape = ke4.shape.displace( text_shape, text_off_x, text_off_y )
				return text_shape
			end, --{\p1}!_G.ke4.text.to_shape( line, syl.text_stripped )!
			
			bord_to_shape = function( Text_Config, Text, Scale, Bord )
				local Text = Text or "text.bord_to_shape"
				local text_scale = Scale or 1
				if Text_Config.styleref then
					Text_Config = Text_Config.styleref
				end
				local text_shape = ke4.text.to_shape( Text_Config, Text, text_scale )
				local bord_width = Bord or Text_Config.outline
				local bord_shape = ke4.shape.to_outline2(
					ke4.shape.flatten( text_shape, 2 ), bord_width * text_scale / 2, bord_width * text_scale / 2
				)
				bord_shape = ke4.shape.ASSDraw3( bord_shape )
				if bord_shape ~= "" then
					return bord_shape
				end
				return ""
			end, --{\p1}!_G.ke4.text.bord_to_shape( line.styleref, line.text_stripped, 1, 3 )!

			deformed = function( Text_Config, Text, Deformed, Pixel, Axis )
				local Axis  = Axis or "x"
				local Pixel = Pixel or 20--l.height
				local Deformed = Deformed or 4
				local DeformeD, PixeL = Deformed, Pixel
				if type( Axis ) == "table" then
					DeformeD = Axis[ 1 ]
					PixeL = Axis[ 2 ]
				end
				local text_def  = Text or "text.deformed"
				local text_shp1 = ke4.text.to_shape( Text_Config, text_def, 1, true )
				if text_shp1 ~= "" then
					local text_fltr = function( x, y )
						local px, py = x, y
						if Axis == "x" then
							y = y - Pixel * sin( ((px - minx) / w_shape) * Deformed * pi )
						elseif Axis == "y" then
							x = x + Pixel * sin( ((py - miny) / h_shape) * Deformed * pi )
						else
							x = x + PixeL * sin( ((py - miny) / h_shape) * DeformeD * pi )
							y = y - Pixel * sin( ((px - minx) / w_shape) * Deformed * pi )
						end
						return x, y
					end
					local text_shp2 = ke4.shape.filter2( text_shp1, text_fltr, 6 )
					return text_shp2
				end
				return ""
			end, --{\p1}!_G.ke4.text.deformed( line.styleref, line.text_stripped, 8, 3, "x" )!
			
			deformed2 = function( Text_Config, Text, Mode )
				local Text = Text or "text.deformed2"
				local text_shape = ke4.text.to_shape( Text_Config, Text, 8, true )
				if text_shape ~= "" then
					ke4.shape.info( text_shape )
					local center_dx = minx + w_shape / 2
					local center_dy = miny + h_shape / 2
					if Mode == nil
						or Mode == 1 then
						local k = 0
						text_shape = ke4.shape.filtery( ke4.shape.redraw( text_shape, 6 ), 
							function( x, y )
								local def_angle = ke4.math.angle( center_dx, center_dy, x, y )
								k = k + 1
								x = center_dx + ke4.math.polar( def_angle, 320 + 10 * (-1) ^ k, "x" )
								y = center_dy + ke4.math.polar( def_angle, 320 + 10 * (-1) ^ k, "y" )
								return x, y
							end
						)
					elseif Mode == 2 then
						text_shape = ke4.shape.filtery( ke4.shape.redraw( text_shape, 6 ), 
							function( x, y )
								local def_angle = ke4.math.angle( center_dx, center_dy, x, y )
								local des_dista = ke4.math.distance( center_dx, center_dy, x, y )
								local des_radiu = ((des_dista <= 80) and ((des_dista <= 60) and 70 or 140) or 200)
								x = center_dx + ke4.math.polar( def_angle, des_radiu, "x" )
								y = center_dy + ke4.math.polar( def_angle, des_radiu, "y" )
								return x, y
							end
						)
					elseif Mode == 3 then
						text_shape = ke4.shape.filtery( ke4.shape.redraw( text_shape, 6 ), 
							function( x, y )
								local def_angle = ke4.math.angle( center_dx, center_dy, x, y )
								local def_angRE = (def_angle - 1) % 60 + 1
								local def_ang3A = 180 - 60 - def_angRE
								local des_radiu = 200
								local des_dista = des_radiu * sin( rad( 60 ) ) / sin( rad( def_ang3A ) )
								local des_radDE = ((ke4.math.distance( center_dx, center_dy, x, y ) <= 80) and des_dista / 2 or des_dista)
								x = center_dx + ke4.math.polar( def_angle, des_radDE, "x" )
								y = center_dy + ke4.math.polar( def_angle, des_radDE, "y" )
								return x, y
							end
						)
					end
					return format( "{\\p4}%s", ke4.shape.ASSDraw3( text_shape ) )
				end
				return ""
			end, --!_G.ke4.text.deformed2( line, syl.text_stripped, 3 )!
			
			to_clip = function( Text_Config, Text, relative_pos, iclip, Scale )
				local Text = Text or "text.to_clip"
				local text_scale = Scale or 1
				local text_clip = ke4.text.to_shape( Text_Config, Text, text_scale, true )
				local text_width, text_height
				local text_mode = ""
				if Text_Config.styleref then
					--permite que el primer parámetro sea simplemente <line>
					Text_Config = Text_Config.styleref
				end
				if text_clip ~= "" then
					text_width, text_height, descent = aegisub.text_extents( Text_Config, Text )
					text_clip = ke4.shape.displace( text_clip, relative_pos[ 1 ] + 0.5 * text_width * (1 - text_scale), relative_pos[ 2 ] + 0.5 * text_height * (1 - text_scale) )
					--text_clip = ke4.shape.displace( text_clip, relative_pos[ 1 ], relative_pos[ 2 ] )
					if iclip then
						text_mode = "i"
					end
					return format( "\\%sclip(%s)", text_mode, text_clip )
				end
				return ""
			end, --{!_G.ke4.text.to_clip( line, line.text_stripped, { line.left, line.top } )!}
			
			to_pixels = function( Text_Config, Text, Ratio, Return_pos )
				local text_2pixel = Text or "text.to_pixels"
				local Ratio = Ratio or 1
				local pixels, pixel_datas, pixel_pos = { }, { }, { }
				if ke4.text.to_shape( Text_Config, text_2pixel ) == "" then
					return ""
				end
				pixel_table = ke4.shape.to_pixels2( ke4.shape.ratio( ke4.text.to_shape( Text_Config, text_2pixel, 1, true ), Ratio ) )
				if Text_Config.styleref then
					--permite que el primer parámetro sea simplemente <line>
					Text_Config = Text_Config.styleref
				end
				local text_width, text_height = aegisub.text_extents( Text_Config, text_2pixel )
				for i = 1, #pixel_table do
					pixel_datas[ i ] = { }
					for k, v in pairs( pixel_table[ i ] ) do
						table.insert( pixel_datas[ i ], v )
					end
				end
				for i = 1, #pixel_table do
					pixels[ i ] = {
						x = pixel_datas[ i ][ 2 ] - 0.5 * Ratio * text_width,
						y = pixel_datas[ i ][ 1 ] - 0.5 * Ratio * text_height,
						a = ke4.alpha.val2ass( 255 - pixel_datas[ i ][ 3 ] )
					}
					pixel_pos[ i ] = {
						x = ke4.math.round( pixel_datas[ i ][ 2 ] - 0.5 * Ratio * text_width ),
						y = ke4.math.round( pixel_datas[ i ][ 1 ] - 0.5 * Ratio * text_height )
					}
				end
				if Return_pos then
					return pixel_pos, pixels
				end
				return pixels
				-- code syl: pixels = _G.ke4.text.to_pixels( line, syl.text_stripped )
				-- template syl notext noblank
			end, --!maxloop( #pixels )!{\an5\pos(!$x + pixels[ j ].x!,!$y + pixels[ j ].y!)\1a!pixels[ j ].a!\bord0\shad0\p1}m 0 0 l 0 1 l 1 1 l 1 0 
			
			image = function( Text_Config, Text, bmp_image )
				local text_image = Text or "text.image"
				local bmp_image = bmp_image or "test.bmp"
				local px_text_pos, px_text_sett = ke4.text.to_pixels( Text_Config, text_image, nil, true )
				local px_image_pos, px_image_sett = ke4.image.to_pixels( bmp_image, nil, true )
				local pixels, ix = { }
				for i = 1, #px_image_sett do
					if type( px_text_pos ) == "table" then
						if ke4.table.inside( px_text_pos, px_image_pos[ i ] ) then
							ix = ke4.table.index( px_text_pos, px_image_pos[ i ] )
							pixels[ #pixels + 1 ] = {
								x = px_text_sett[ ix ].x,
								y = px_text_sett[ ix ].y,
								a = px_image_sett[ i ].a,
								c = px_image_sett[ i ].c
							}
						end
					end
				end
				return pixels
				--code syl: pixels = _G.ke4.text.image( line, syl.text_stripped, "test.bmp" )
				--!maxloop( #pixels )!{\an5\pos(!$x + pixels[ j ].x!,!$y + pixels[ j ].y!)\1c!pixels[ j ].c!\1a!pixels[ j ].a!\bord0\shad0\p1}m 0 0 l 0 1 l 1 1 l 1 0 
			end,
			
			bord_to_pixels = function( Text_Config, Text, Pixel )
				local text_2bord = Text or "text.bord_to_pixels"
				local size_pixel = Pixel or 2
				local text_shape = ke4.text.to_shape( Text_Config, text_2bord, 1, true )
				local points = ke4.shape.point( text_shape, size_pixel )
				if Text_Config.styleref then
					--permite que el primer parámetro sea simplemente <line>
					Text_Config = Text_Config.styleref
				end
				local text_width, text_height = aegisub.text_extents( Text_Config, text_2bord )
				for i = 1, #points do
					points[ i ].x = points[ i ].x - 0.5 * text_width
					points[ i ].y = points[ i ].y - 0.5 * text_height
				end
				return points
				-- code line: points = _G.ke4.text.bord_to_pixels( line, line.text_stripped )
				-- template syl notext noblank
			end, --!maxloop( #points )!{\an5\pos(!$x + points[ j ].x!,!$y + points[ j ].y!)\bord0\shad0\p1}m 0 0 l 0 1 l 1 1 l 1 0 
			
			filter = function( Text_Config, Text, Split, ... )
				local txt_shape = ke4.text.to_shape( Text_Config, Text, 1, true )
				local Split = Split or 3
				return ke4.shape.filter3( txt_shape, Split, ... )
			end, --{\p1}!_G.ke4.text.filter( line, line.text_stripped, 3, function( x, y ) x = x + _G.Rcs( 2 ) y = y + _G.Rcs( 2 ) return x, y end )!

			gradienth = function( Text_Config, Text, Relative_pos, ... )
				local shp_w = 2
				if Text_Config.styleref then
					--permite que el primer parámetro sea simplemente <line>
					Text_Config = Text_Config.styleref
				end
				local Width, Height = aegisub.text_extents( Text_Config, Text )
				local Shape, cn = "", ceil( Width / shp_w )
				local gradh = ke4.table.gradient( { { cn } }, ... )
				for i = 1, cn do
					Shape = Shape .. format( "{\\1c%s\\p1}%s", gradh[ i ], ke4.shape.size( ke4.shape.rectangle, shp_w, ceil( Height ) ) )
				end
				local Rel_pos = Relative_pos
				Rel_pos[ 1 ] = Rel_pos[ 1 ] - 0.5 * Width
				Rel_pos[ 2 ] = Rel_pos[ 2 ] - 0.5 * Height
				return format( "{%s\\bord0\\shad0}%s", ke4.text.to_clip( Text_Config, Text, Rel_pos ), Shape )
			end, --!_G.ke4.text.gradienth( line, line.text_stripped, { line.center, line.middle }, "&H00FFFF&", "&H0000FF&" )!
			
			gradientv = function( Text_Config, Text, Relative_pos, ... )
				local shp_h = 2
				if Text_Config.styleref then
					--permite que el primer parámetro sea simplemente <line>
					Text_Config = Text_Config.styleref
				end
				local Width, Height = aegisub.text_extents( Text_Config, Text )
				local Shape, cn = "", ceil( Height / shp_h )
				local gradv = ke4.table.gradient( { { cn } }, ... )
				for i = 1, cn do
					Shape = Shape .. format( "{\\1c%s\\p1}%s{\\p0}\\N", gradv[ i ], ke4.shape.size( ke4.shape.rectangle, ceil( Width ), shp_h ) )
				end
				local Rel_pos = Relative_pos
				Rel_pos[ 1 ] = Rel_pos[ 1 ] - 0.5 * Width
				Rel_pos[ 2 ] = Rel_pos[ 2 ] - 0.5 * Height
				return format( "{%s\\bord0\\shad0}%s", ke4.text.to_clip( Text_Config, Text, Rel_pos ), Shape )
			end, --!_G.ke4.text.gradientv( line, line.text_stripped, { line.center, line.middle }, "&H00FFFF&", "&H0000FF&" )!
			
			gradientangle = function( Text_Config, Text, Relative_pos, Angle, ... )
				local shp_s = 2
				local Angle = Angle or 0
				if Text_Config.styleref then
					--permite que el primer parámetro sea simplemente <line>
					Text_Config = Text_Config.styleref
				end
				local Width, Height = aegisub.text_extents( Text_Config, Text )
				local shp_w = ke4.math.round( abs( Width * cos( rad( Angle ) ) + Height * sin( rad( Angle ) ) + 1 ) )
				local shp_h = ke4.math.round( abs( Width * sin( rad( Angle ) ) + Height * cos( rad( Angle ) ) + 1 ) )
				local Shape, cn = format( "{\\fr%s}", Angle % 361 ), ceil( shp_w / shp_s )
				local grada = ke4.table.gradient( { { cn } }, ... )
				for i = 1, cn do
					Shape = Shape .. format( "{\\1c%s\\p1}%s", grada[ i ], ke4.shape.size( ke4.shape.rectangle, shp_s, shp_h ) )
				end
				local Rel_pos = Relative_pos
				Rel_pos[ 1 ] = Rel_pos[ 1 ] - 0.5 * Width
				Rel_pos[ 2 ] = Rel_pos[ 2 ] - 0.5 * Height
				return format( "{%s\\bord0\\shad0}%s", ke4.text.to_clip( Text_Config, Text, Rel_pos ), Shape )
			end, --!_G.ke4.text.gradientangle( line, line.text_stripped, { line.center, line.middle }, 45, "&H00FFFF&", "&H0000FF&" )!
			
			bezier = function( Text_Config, Shape, Char_x, Char_y, Mode, Offset )
				local pyointa = { }
				function pyointa.tangential2P( Pnts, t_ )
					local tanVec, XY, dpos = { }, { }, { }
					XY = pyointa.difference( Pnts )
					dpos = pyointa.tDifferential( XY, t_ )
					for i = 1, 2 do
						tanVec[ i ] = dpos[ 2 ][ i ] / math.sqrt( dpos[ 2 ][ 1 ] ^ 2 + dpos[ 2 ][ 2 ] ^ 2 )
					end
					return tanVec
				end

				function pyointa.normal2P( Pnts, t_ )
					local normalVec = { }
					normalVec = pyointa.tangential2P( Pnts, t_ )
					normalVec[ 1 ], normalVec[ 2 ] = normalVec[ 2 ], -normalVec[ 1 ]
					return normalVec
				end

				function pyointa.difference( Pnts )
					local DVec, XY = { }, { }
					-- 1st step difference
					DVec[ 1 ] = { [ 1 ] = Pnts[ 2 ][ 1 ] - Pnts[ 1 ][ 1 ], [ 2 ] = Pnts[ 2 ][ 2 ] - Pnts[ 1 ][ 2 ] }
					DVec[ 2 ] = { [ 1 ] = Pnts[ 3 ][ 1 ] - Pnts[ 2 ][ 1 ], [ 2 ] = Pnts[ 3 ][ 2 ] - Pnts[ 2 ][ 2 ] }
					DVec[ 3 ] = { [ 1 ] = Pnts[ 4 ][ 1 ] - Pnts[ 3 ][ 1 ], [ 2 ] = Pnts[ 4 ][ 2 ] - Pnts[ 3 ][ 2 ] }
					-- 2nd step difference
					DVec[ 4 ] = { [ 1 ] = DVec[ 2 ][ 1 ] - DVec[ 1 ][ 1 ], [ 2 ] = DVec[ 2 ][ 2 ] - DVec[ 1 ][ 2 ] }
					DVec[ 5 ] = { [ 1 ] = DVec[ 3 ][ 1 ] - DVec[ 2 ][ 1 ], [ 2 ] = DVec[ 3 ][ 2 ] - DVec[ 2 ][ 2 ] }
					-- 3rd step difference
					DVec[ 6 ] = { [ 1 ] = DVec[ 5 ][ 1 ] - DVec[ 4 ][ 1 ], [ 2 ] = DVec[ 5 ][ 2 ] - DVec[ 4 ][ 2 ] }
					XY[ 1 ] = { [ 1 ] = Pnts[ 1 ][ 1 ], [ 2 ] = Pnts[ 1 ][ 2 ] }
					XY[ 2 ] = { [ 1 ] = DVec[ 1 ][ 1 ], [ 2 ] = DVec[ 1 ][ 2 ] }
					XY[ 3 ] = { [ 1 ] = DVec[ 4 ][ 1 ], [ 2 ] = DVec[ 4 ][ 2 ] }
					XY[ 4 ] = { [ 1 ] = DVec[ 6 ][ 1 ], [ 2 ] = DVec[ 6 ][ 2 ] }
					return XY
				end

				function pyointa.tDifferential( XY, ta ) 
					local dPos = { }
					dPos[ 1 ] = {
						[ 1 ] = XY[ 4 ][ 1 ] * ta ^ 3 + 3 * XY[ 3 ][ 1 ] * ta ^ 2 + 3 * XY[ 2 ][ 1 ] * ta + XY[ 1 ][ 1 ],
						[ 2 ] = XY[ 4 ][ 2 ] * ta ^ 3 + 3 * XY[ 3 ][ 2 ] * ta ^ 2 + 3 * XY[ 2 ][ 2 ] * ta + XY[ 1 ][ 2 ]
					}
					dPos[ 2 ] = {
						[ 1 ] = 3 * (XY[ 4 ][ 1 ] * ta ^ 2 + 2 * XY[ 3 ][ 1 ] * ta + XY[ 2 ][ 1 ]),
						[ 2 ] = 3 * (XY[ 4 ][ 2 ] * ta ^ 2 + 2 * XY[ 3 ][ 2 ] * ta + XY[ 2 ][ 2 ])
					}
					dPos[ 3 ] = {
						[ 1 ] = 6 * (XY[ 4 ][ 1 ] * ta + XY[ 3 ][ 1 ]),
						[ 2 ] = 6 * (XY[ 4 ][ 2 ] * ta + XY[ 3 ][ 2 ])
					}
					return dPos
				end

				function pyointa.getBezierLength( p, ta , tb, nN ) 
					local XY, dpos, t_ = { }, { }, { }
					for i = 1, 2 * nN + 1 do
						t_[ i ] = ta + (i - 1) * (tb - ta) / (2 * nN)
					end
					XY = pyointa.difference( p )				
					dpos = pyointa.tDifferential( XY, t_[ 1 ] )	
					local Ft1 = (dpos[ 2 ][ 1 ] ^ 2 + dpos[ 2 ][ 2 ] ^ 2) ^ 0.5
					dpos = pyointa.tDifferential( XY, t_[ 2 * nN + 1 ] )
					local Ft2 = (dpos[ 2 ][ 1 ] ^ 2 + dpos[ 2 ][ 2 ] ^ 2) ^ 0.5
					local SFt1 = 0
					for i = 1, nN do
						dpos = pyointa.tDifferential( XY, t_[ 2 * i ] )
						SFt1 = SFt1 + (dpos[ 2 ][ 1 ] ^ 2 + dpos[ 2 ][ 2 ] ^ 2) ^ 0.5
					end
					local SFt2 = 0
					for i = 1, nN - 1 do
						dpos = pyointa.tDifferential( XY, t_[ 2 * i + 1 ] )
						SFt2 = SFt2 + (dpos[ 2 ][ 1 ] ^ 2 + dpos[ 2 ][ 2 ] ^ 2) ^ 0.5
					end
					local SimpLength = ((tb - ta) / (2 * nN) / 3) * ((Ft1 + Ft2) + (4 * SFt1) + (2 * SFt2))
					return SimpLength
				end

				function pyointa.length2t( Pnts, Ltarget, nN ) 
					local ll = { [ 1 ] = 0 }
					local ni, tb, t_ = 1.0 / nN, 0, 0
					for i = 2, nN + 1 do
						tb = tb + ni
						ll[ i ] = pyointa.getBezierLength( Pnts, 0, tb, nN * 2 )
					end
					if Ltarget > ll[ nN + 1 ] then
						t_ = false
						return t_
					end
					for i = 1, nN do
						if ((Ltarget >= ll[ i ])
							and (Ltarget <= ll[ i + 1 ])) then
							t_ = (i - 1) / nN + (Ltarget - ll[ i ]) / (ll[ i + 1 ] - ll[ i ]) * (1 / nN)
							break
						end
					end
					return t_
				end

				function pyointa.length2PtNo( Pnts, Ltarget, nN )
					local bl, cpoint = { }, { }
					local leng
					for h = 1, #Pnts do
						bl = { }
						bl[ 1 ] = 0
						for i = 2, #Pnts[ h ] + 1 do
							bl[ i ] = bl[ i - 1 ] + pyointa.getBezierLength( Pnts[ h ][ i - 1 ], 0, 1.0, nN )
						end
						if Ltarget > bl[ #bl ] then
							Ltarget = Ltarget - bl[ #bl ]
						else
							for k = 1, #Pnts[ h ] do
								if ((Ltarget >= bl[ k ])
									and (Ltarget <= bl[ k + 1 ])) then
									cpoint = Pnts[ h ][ k ]
									leng = Ltarget - bl[ k ]
									break
								end
							end
						end
						if leng then
							break
						end
					end
					if leng then
						return cpoint, leng
					end
					return false
				end

				function pyointa.getBezierPos( Pnts, t_ ) 
					local XY, pos_Bzr = { }, { }
					--local t_ = t_ or 1
					XY = pyointa.difference( Pnts )
					for i = 1, 2 do
						pos_Bzr[ i ] = XY[ 4 ][ i ] * t_ ^ 3 + 3 * XY[ 3 ][ i ] * t_ ^ 2 + 3 * XY[ 2 ][ i ] * t_ + XY[ 1 ][ i ]
					end
					return pos_Bzr
				end

				function pyointa.shape2coord( Shape ) 
					local coord, xy, k = { }, { }, 0
					for c in Shape:gmatch( "%S+" ) do
						table.insert( xy, c )
					end
					repeat
						k = k + 1
					until xy[ k ] == "m" or k > #xy
					if k > 1 then
						aegisub.debug.out( "invalid drawing command" )
					end
					local d_comm = "m"
					local i = 1
					k = k + 3
					coord[ i ] = { }
					while k < #xy do 
						if xy[ k ] == "m" then 
							k = k + 3
							i = i + 1
							coord[ i ] = { }
							d_comm = "m"
						elseif xy[ k ] == "b" then
							cp1x, cp1y = xy[ k - 2 ], xy[ k - 1 ] 
							cp2x, cp2y = xy[ k + 1 ], xy[ k + 2 ]
							cp3x, cp3y = xy[ k + 3 ], xy[ k + 4 ]
							cp4x, cp4y = xy[ k + 5 ], xy[ k + 6 ]
							k = k + 7
							d_comm = "b"
							table.insert( coord[ i ], { { cp1x, cp1y }, { cp2x, cp2y }, { cp3x, cp3y }, { cp4x, cp4y } } )
						elseif xy[ k ] == "l" then 		
							cp1x, cp1y = xy[ k - 2 ], xy[ k - 1 ]
							cp2x = xy[ k - 2 ] + ((xy[ k + 1 ] - xy[ k - 2 ]) * (1 / 3))
							cp2y = xy[ k - 1 ] + ((xy[ k + 2 ] - xy[ k - 1 ]) * (1 / 3))
							cp3x = xy[ k - 2 ] + ((xy[ k + 1 ] - xy[ k - 2 ]) * (2 / 3))
							cp3y = xy[ k - 1 ] + ((xy[ k + 2 ] - xy[ k - 1 ]) * (2 / 3))
							cp4x, cp4y = xy[ k + 1 ], xy[ k + 2 ]
							k = k + 3
							d_comm = "l"
							table.insert( coord[ i ], { { cp1x, cp1y }, { cp2x, cp2y }, { cp3x, cp3y }, { cp4x, cp4y } } )
						elseif string.match( xy[ k ], "%d+" ) ~= nil then
							if d_comm == "b" then        
								cp1x, cp1y = xy[ k - 2 ], xy[ k - 1 ] 
								cp2x, cp2y = xy[ k + 0 ], xy[ k + 1 ]
								cp3x, cp3y = xy[ k + 2 ], xy[ k + 3 ]
								cp4x, cp4y = xy[ k + 4 ], xy[ k + 5 ]
								k = k + 6
								table.insert( coord[ i ], { { cp1x, cp1y }, { cp2x, cp2y }, { cp3x, cp3y }, { cp4x, cp4y } } )
							elseif d_comm == "l" then       
								cp1x, cp1y = xy[ k - 2 ], xy[ k - 1 ]
								cp2x = xy[ k - 2 ] + ((xy[ k + 0 ] - xy[ k - 2 ]) * (1 / 3))
								cp2y = xy[ k - 1 ] + ((xy[ k + 1 ] - xy[ k - 1 ]) * (1 / 3))
								cp3x = xy[ k - 2 ] + ((xy[ k + 0 ] - xy[ k - 2 ]) * (2 / 3))
								cp3y = xy[ k - 1 ] + ((xy[ k + 1 ] - xy[ k - 1 ]) * (2 / 3))
								cp4x, cp4y = xy[ k ], xy[ k + 1 ]
								k = k + 2     
								table.insert( coord[ i ], { { cp1x, cp1y }, { cp2x, cp2y }, { cp3x, cp3y }, { cp4x, cp4y } } )
							end
						else
							aegisub.debug.out( "unkown drawing command" )
						end
					end
					return coord
				end
				
				local line_info = ke4.ass.linefx( Text_Config )
				local l_width, l_left, l_descent = line_info.width, line_info.left, line_info.descent
				if Shape == nil then
					if line_info.text:match( "\\i?clip%b()" ) then
						Shape = line_info.text:match( "\\i?clip%b()" )
					end
				end
				local pos_Bezier, vec_Bezier, cont_point, PtNo = { }, { }, { }, { }
				local nN, Blength, lineoffset = 8, 0, 0
				cont_point = pyointa.shape2coord( ke4.shape.ASSDraw3( Shape ) )
				for i = 1, #cont_point do
					for k = 1, #cont_point[ i ] do
						Blength = Blength + pyointa.getBezierLength( cont_point[ i ][ k ], 0 , 1.0, nN )
					end
				end
				local Offset = Offset or 0
				if Mode == 2 then -- alinea el texto desde la izquierda
					lineoffset = Offset
				elseif Mode == 3 then -- alinea el texto desde la derecha
					lineoffset = Blength - l_width - Offset
				elseif Mode == 4 then
					--si Offset = (ci - 1) / (cn - 1) justifica el texto en toda la longitud de la shape, equidistantemente
					--Offset = (j - 1) / (maxj - 1) anima el texto de inicio a fin de la shape
					--!maxloop( orgline.duration / frame_dur )!
					--!retime( "preline", frame_dur * (j - 1), frame_dur * j )!
					lineoffset = (Blength - l_width) * Offset
				elseif Mode == 5 then -- anima el texto de fin a inicio de la shape
					--!maxloop( orgline.duration / frame_dur )!!retime( "preline", frame_dur * (j - 1), frame_dur * j )!{\an5!_G.ke4.text.bezier( line, nil, $x, $y, 5, (j - 1) / (maxj - 1) )!}
					lineoffset = (Blength - l_width) * (1 - Offset)
				else --(Mode == 1) modo por default (centro de la shape)
					lineoffset = (Blength - l_width) / 2 + Offset
				end
				targetLength, rot_Bezier = 0, 0
				PtNo, targetLength = pyointa.length2PtNo( cont_point, lineoffset + Char_x - l_left, nN )
				if PtNo ~= false then
					tb = pyointa.length2t( PtNo, targetLength, nN )
					if tb ~= false then
						pos_Bezier = pyointa.getBezierPos( PtNo, tb )
						vec_Bezier = pyointa.normal2P( PtNo, tb )
						rot_Bezier = -deg( atan2( vec_Bezier[ 2 ], vec_Bezier[ 1 ] ) ) - 90
					end
				else
					pos_Bezier[ 1 ] = Char_x
					pos_Bezier[ 2 ] = Char_y-- + l_descent
					rot_Bezier = 0
				end
				bezier_angle = ke4.math.round( ke4.tag.only( rot_Bezier < -180, rot_Bezier + 360, rot_Bezier ), 3 )
				return format( "\\pos(%s,%s)\\fr%s", ke4.math.round( pos_Bezier[ 1 ], 3 ), ke4.math.round( pos_Bezier[ 2 ], 3 ), bezier_angle )
			end, --{\an5!_G.ke4.text.bezier( line, "\\clip(m 180 352 b 303 327 347 239 464 236 589 239 595 474 812 450 987 428 871 261 1144 308)", $x, $y, nil, 0 )!}
			
			rand = function( Text_Config, Text, num_tran, dur_tran, Tags, Table, Rand, All )
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local Text = Text or "ke4.text.rand"
				local dur_tran = abs( dur_tran or 2 * frame_dur )
				local num_tran = abs( ke4.math.round( num_tran or 5 ) )
				local del_tran = 0
				local delay_tr = dur_tran * num_tran
				----------------------------------------------
				if dur_tran < frame_dur then
					dur_tran = frame_dur
				end
				if delay_tr == 0 or
					delay_tr > Text_Config.duration then
					delay_tr = Text_Config.duration
					num_tran = ceil( Text_Config.duration / dur_tran )
				end
				----------------------------------------------
				local table_ch = { }
				for i = 48, 57 do --dígitos
					table.insert( table_ch, string.char( i ) )
				end
				for i = 65, 90 do --minúsculas y mayúsculas
					table.insert( table_ch, string.char( i ) )
					table.insert( table_ch, string.char( i + 32 ) )
				end
				local tbl_rand = Table or table_ch
				----------------------------------------------
				local extra_tg = Tags or ""
				local time_ini = ke4.math.R( 0, Text_Config.duration - delay_tr, 5 * frame_dur )
				---------------------------------------------------------
				if Rand == "intro"
					or Rand == "line" then
					time_ini = 0
				elseif Rand == "outro" then
					time_ini = Text_Config.duration - delay_tr
				end
				---------------------------------------------------------
				local tbl_char = ke4.table.string( Text )
				local tbl_rtrn = { }
				local time_line = Text_Config.duration - delay_tr
				local l = Text_Config.styleref
				---------------------------------------------------------
				local Ad = format( "\\1a%s\\3a%s\\4a%s", ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color3 ), ke4.alpha.fromstyle( l.color4 ) )
				local Ai = "\\1a&HFF&\\3a&HFF&\\4a&HFF&"
				if l.outline == 0
					and l.shadow == 0 then
					Ad = format( "\\1a%s", ke4.alpha.fromstyle( l.color1 ) )
					Ai = "\\1a&HFF&"
				elseif l.shadow == 0 then
					Ad = format( "\\1a%s\\3a%s", ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color3 ) )
					Ai = "\\1a&HFF&\\3a&HFF&"
				elseif l.outline == 0 then
					Ad = format( "\\1a%s\\4a%s", ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color4 ) )
					Ai = "\\1a&HFF&\\4a&HFF&"
				end
				for i = 1, #tbl_char do
					if tbl_char[ i ] ~= " " then
						if ke4.table.inside( ke4.text.char_special, tbl_char[ i ] ) then
							tbl_rtrn[ i ] = format( "{\\fscx%s}%s", l.scale_x, tbl_char[ i ] )
						else
							if Rand == "line" then
								tbl_rtrn[ i ] = format( "{\\fscx%s%s\\t(%s,%s,\\fscx0%s)\\t(%s,%s,\\fscx%s%s)\\t(%s,%s,\\fscx0%s)}%s",
									l.scale_x, Ad, time_ini, time_ini + del_tran, Ai, time_ini + delay_tr, time_ini + delay_tr + del_tran,
									l.scale_x, Ad, time_line, time_line + del_tran, Ai, tbl_char[ i ]
								)
								for k = 1, num_tran do
									tbl_rtrn[ i ] = tbl_rtrn[ i ] .. format( "{\\fscx0%s\\t(%s,%s,\\fscx%s%s%s)\\t(%s,%s,\\fscx0%s%s)\\t(%s,%s,\\fscx%s%s)\\t(%s,%s,\\fscx0%s)}%s",
										Ai, time_ini + (k - 1) * dur_tran, time_ini + del_tran + (k - 1) * dur_tran, l.scale_x, Ad, extra_tg,
										time_ini + (k - 0) * dur_tran, time_ini + del_tran + (k - 0) * dur_tran, Ai, ke4.tag.default( Text_Config, extra_tg ),
										time_line + (k - 1) * dur_tran, time_line + del_tran + (k - 1) * dur_tran, l.scale_x, Ad,
										time_line + (k - 0) * dur_tran, time_line + del_tran + (k - 0) * dur_tran, Ai, ke4.math.Re( tbl_rand )
									)
								end
							else
								tbl_rtrn[ i ] = format( "{\\fscx%s%s\\t(%s,%s,\\fscx0%s)\\t(%s,%s,\\fscx%s%s)}%s",
									l.scale_x, Ad, time_ini, time_ini + del_tran, Ai, time_ini + delay_tr,
									time_ini + delay_tr + del_tran, l.scale_x, Ad, tbl_char[ i ]
								)
								for k = 1, num_tran do
									tbl_rtrn[ i ] = tbl_rtrn[ i ] .. format( "{\\fscx0%s\\t(%s,%s,\\fscx%s%s%s)\\t(%s,%s,\\fscx0%s%s)}%s",
										Ai, time_ini + (k - 1) * dur_tran, time_ini + del_tran + (k - 1) * dur_tran, l.scale_x, Ad, extra_tg,
										time_ini + (k - 0) * dur_tran, time_ini + del_tran + (k - 0) * dur_tran, Ai, ke4.tag.default( Text_Config, extra_tg ), ke4.math.Re( tbl_rand )
									)
								end
							end
						end
					end
				end
				local Text_fx = ke4.tag.dark( Text_Config, table.concat( tbl_rtrn ) )
				--local Text_fx = table.concat( tbl_rtrn )
				--------------------------------------------------------------
				Text_fx = Text_fx:gsub( "\\t(%b())",
					function( capture )
						if capture:sub( 2, -2 ):sub( 1, 4 ) == "0,0," then
							return capture:sub( 2, -2 ):match( "\\%S+[ %S]*" )
							--captura todos los tags dentro de una \\t
						end
					end
				) --si hay una \\t(0,0, solo retorna los tags que hay dentro
				--------------------------------------------------------------
				if All
					or Rand == "intro"
					or Rand == "line"
					or Rand == "outro" then
					return Text_fx--table.concat( tbl_rtrn )
				end --char.text:rand( 5, 2f, "\\1cR( )" )
				return ke4.tag.only( ke4.math.R( ke4.math.R( 2, 4 ) ) == 1, Text_fx, Text )
			end, --{\an5\pos($x,$y)}!_G.ke4.text.rand( line, syl.text, 5, 82 )!
			
			upper = function( Text )
				local Text = Text or "_G.ke4.text.upper"
				local up_tag2, up_tags = { }, { }
				for c in Text:gmatch( "%b{}" ) do
					table.insert( up_tag2, c )
				end
				local txt_2up = unicode.to_upper_case( Text )
				for c in txt_2up:gmatch( "%b{}" ) do
					table.insert( up_tags, c )
				end
				for i = 1, #up_tags do
					txt_2up = txt_2up:gsub( ke4.tag.operation( up_tags[ i ] ), up_tag2[ i ], 1 )
				end
				txt_2up = txt_2up:gsub( "KEfx", "" )
				return txt_2up
			end, --!_G.ke4.text.upper( line.text_stripped )!
			
			lower = function( Text )
				local Text = Text or "_G.ke4.text.lower"
				local lw_tag2, lw_tags = { }, { }
				for c in Text:gmatch( "%b{}" ) do
					table.insert( lw_tag2, c )
				end
				local txt_2lw = unicode.to_lower_case( Text )
				for c in txt_2lw:gmatch( "%b{}" ) do
					table.insert( lw_tags, c )
				end
				for i = 1, #lw_tags do
					txt_2lw = txt_2lw:gsub( ke4.tag.operation( lw_tags[ i ] ), lw_tag2[ i ], 1 )
				end
				txt_2lw = txt_2lw:gsub( "KEfx", "" )
				return txt_2lw
			end, --!_G.ke4.text.upper( line.text_stripped )!
			
			kara = function( Text_Config )
				local text_2kara = ""
				for i = 1, Text_Config.kara.n do
					if i < Text_Config.kara.n
						or Text_Config.kara[ i ].text:gsub( " ", "" ) ~= "" then
						text_2kara = text_2kara .. format( "{\\k%s}%s",
							Text_Config.kara[ i ].kdur, Text_Config.kara[ i ].text
						)
					end
				end
				text_2kara = text_2kara:gsub( "\\k0", "" ):gsub( "{}", "" )
				text_2kara = text_2kara:gsub( "(%b{})(%b{})",
					function( capture1, capture2 )
						local Times, Tag_k
						if capture1:match( "\\[kK]^*[fo]*%d+" )
							and capture2:match( "\\[kK]^*[fo]*%d+" ) then
							Times = capture1:match( "\\[kK]^*[fo]*(%d+)" ) + capture2:match( "\\[kK]^*[fo]*(%d+)" )
							Tag_k = capture2:match( "\\([kK]^*[fo]*)%d+" )
							capture1 = capture1:sub( 2, -2 ):gsub( "\\[kK]^*[fo]*%d+", "" )
							capture2 = capture2:sub( 2, -2 ):gsub( "\\[kK]^*[fo]*%d+", "" )
							return format( "{%s%s\\%s%d}", capture1, capture2, Tag_k, Times )
						end
						return format( "{%s%s}", capture1:sub( 2, -2 ), capture2:sub( 2, -2 ) )
					end
				)
				text_2kara = text_2kara:gsub( "%b{} ",
					function( capture )
						capture = capture:sub( 1, -2 )
						return " " .. capture .. "|"
					end
				)
				return text_2kara
			end, --!_G.ke4.text.kara( line )!
			
			to_kara = function( Text, Kmode )
				local Kmode = Kmode or "k"
				local K_mode_tbl = { "k", "kf", "ko" }
				if ke4.table.inside( K_mode_tbl, Kmode ) == false then
					Kmode = "k"
				end
				local romajis = {
					----------------------------------------------------------------------
					"kya",	"kyu",	"kyo",	"sha",	"shu",	"sho",	"cha",	"chu",	"cho",
					"nya",	"nyu",	"nyo",	"hya",	"hyu",	"hyo",	"mya",	"myu",	"myo",
					"rya",	"ryu",	"ryo",	"gya",	"gyu",	"gyo",	"bya",	"byu",	"byo",
					"pya",	"pyu",	"pyo",	"shi",	"chi",	"tsu",
					"ya",	"yu",	"yo",	"ka",	"ki",	"ku",	"ke",	"ko",	"sa",
					"su",	"se",	"so",	"ta",	"te",	"to",	"na",	"ni",	"nu",
					"ne",	"no",	"ha",	"hi",	"fu",	"he",	"ho",	"ma",	"mi",
					"mu",	"me",	"mo",	"ya",	"yu",	"yo",	"ra",	"ri",	"ru",
					"re",	"ro",	"wa",	"wi",	"we",	"wo",	"ga",	"gi",	"gu",
					"ge",	"go",	"za",	"ji",	"zu",	"ze",	"zo",	"ja",	"ju",
					"jo",	"da",	"di",	"du",	"de",	"do",	"ba",	"bi",	"bu",
					"be",	"bo",	"pa",	"pi",	"pu",	"pe",	"po",
					"a",	"i",	"u",	"e",	"o",	"n",	"b",	"c",	"d",
					"k",	"p",	"r",	"s",	"t",	"z"
					----------------------------------------------------------------------
				}
				local Text = ke4.text.lower( Text )
				Text = Text:gsub( "%b{}", "" )
				local words, times = ke4.text.text2word( Text )
				local num = 0
				for i = 1, #words do
					for k = 1, #romajis do
						words[ i ] = words[ i ]:gsub( "[\128-\255]*" .. romajis[ k ], "[%1]" )
					end
					words[ i ] = words[ i ]:gsub( "%b[]",
						function( capture )
							capture = capture:gsub( "%[", "" ):gsub( "%]", "" )
							return "[" .. capture .. "]"
						end
					)
					words[ i ], num = words[ i ]:gsub( "%b[]", "%1" )
					if num > 0 then
						words[ i ] = words[ i ]:gsub( "%b[]",
							function( capture )
								capture = capture:gsub( "%[", "" ):gsub( "%]", "" )
								return format( "{\\%s%d}%s", Kmode, times[ i ] / (num * 10), capture )
							end
						)
					end
				end
				Text = ke4.table.op( words, "concat" )
				return Text
			end, --!_G.ke4.text.to_kara( line.text_stripped )!
			
			inside = function( Text_Config, Count, Mark )
				--encuentra marcas en las syls y retorna true en el caso de que sí
				local Mark = Mark or "my_mark"
				local inmark = { }
				for i = 1, Text_Config.kara.n do
					if Text_Config.kara[ i ].text:match( Mark ) then
						table.insert( inmark, i )
						if Mark == "+fx" then
							local k = 1
							while Text_Config.kara[ i + k ]
								and not Text_Config.kara[ i + k ].text:match( "-fx" ) do
								table.insert( inmark, i + k )
								k = k + 1
							end
							if Text_Config.kara[ i + k ]
								and Text_Config.kara[ i + k ].text:match( "-fx" ) then
								table.insert( inmark, i + k )
							end
						end
					end
				end
				if ke4.table.inside( inmark, Count ) == true then
					return true
				end
				return false
			end, --!_G.ke4.text.inside( line, syl.i, "fx" )!
			
			outside = function( Text_Config, Count, Mark )
				--encuentra marcas en las syls y retorna false en el caso de que sí
				local Mark = Mark or "my_mark"
				local inmark = { }
				for i = 1, Text_Config.kara.n do
					if Text_Config.kara[ i ].text:match( Mark ) then
						table.insert( inmark, i )
						if Mark == "+fx" then
							local k = 1
							while Text_Config.kara[ i + k ]
								and not Text_Config.kara[ i + k ].text:match( "-fx" ) do
								table.insert( inmark, i + k )
								k = k + 1
							end
							if Text_Config.kara[ i + k ]
								and Text_Config.kara[ i + k ].text:match( "-fx" ) then
								table.insert( inmark, i + k )
							end
						end
					end
				end
				if ke4.table.inside( inmark, Count ) == true then
					return false
				end
				return true
			end, --!_G.ke4.text.outside( line, syl.i, "fx" )!
			
			tag = function( Text, ... )
				local tags_text = { ... }
				local return_text, TextTag = "", Text
				local char_text, tag_i_ipol, tag_n_ipol = { }, "", ""
				if type( tags_text[ 1 ] ) == "string"
					and tags_text[ 1 ]:gsub( "%b{}", "" ) ~= ""
					and tags_text[ 1 ]:sub( 1, 1 ) ~= "\\" then
					TextTag = tags_text[ 1 ]
					table.remove( tags_text, 1 )
				end
				for c in unicode.chars( TextTag ) do
					table.insert( char_text, c )
				end
				---------------------------------------------------------------
				local function text_ipol( vals, count_i, count_n, without_tag )
					if #vals == 1 then
						vals[ 2 ] = vals[ 1 ]
					end
					local copy_tbl = ke4.table.duplicate( vals )
					local tag_into = ""
					if without_tag then
						tag_into = vals[ 1 ]
						copy_tbl = { }
						if #vals == 2 then
							vals[ 3 ] = vals[ 2 ]
						end
						for i = 2, #vals do
							copy_tbl[ i - 1 ] = vals[ i ]
						end
					end
					---------------------------------------------------
					local tags_ipol, max_ipol = { }, count_n - 1
					if max_ipol == 0 then
						return copy_tbl[ 1 ]
					end
					local function ipol_number( val_1, val_2, pct_ipol )
						return val_1 + (val_2 - val_1) * pct_ipol
					end
					local ipol_function = ipol_number
					if tag_into:match( "\\c" )
						or tag_into:match( "\\%d[%d]*v?c" ) then
						ipol_function = ke4.color.interpolate
					elseif tag_into:match( "\\alpha" )
						or tag_into:match( "\\%d[%d]*v?a" ) then
						ipol_function = ke4.alpha.interpolate
					end
					local ipol_i, ipol_f, pct_ip
					for i = 1, max_ipol do
						ipol_i = copy_tbl[ floor( (i - 1) / (max_ipol / (#copy_tbl - 1)) ) + 1 ]
						ipol_f = copy_tbl[ floor( (i - 1) / (max_ipol / (#copy_tbl - 1)) ) + 2 ]
						pct_ip = floor( (i - 1) % (max_ipol / (#copy_tbl - 1)) ) / (max_ipol / (#copy_tbl - 1))
						tags_ipol[ i ] = ipol_function( ipol_i, ipol_f, pct_ip )
					end --text.tag( { "\\fscy", 100, 200, 50 } ) = text.tag( "\\fscy{ 100, 200, 50 }" )
					tags_ipol[ #tags_ipol + 1 ] = copy_tbl[ #copy_tbl ]
					return tags_ipol[ count_i ]
				end
				---------------------------------------------------------------
				local function str_to_table( String )
					if type( String ) == "string"
						and String:match( "\\%w+%b{}" ) then
						local tags_capture = ke4.tag.operation( String:match( "(\\%w+)%b{}" ) )
						local vals_capture = String:match( "\\%w+(%b{})" )
						if pcall( loadstring( format( "return function( ) return %s end", vals_capture ) ) ) then
							local string_func = loadstring( format( "return function( ) return %s end", vals_capture ) )( )
							if pcall( string_func ) then
								local vals_table = string_func( )
								if vals_table then
									table.insert( vals_table, 1, tags_capture )
									return vals_table
								end
							end
						end
						return String
					end
					return String
				end
				---------------------------------------------------------------
				for i = 1, #char_text do
					tag_n_ipol = ""
					for k = 1, #tags_text do
						tags_text[ k ] = str_to_table( tags_text[ k ] )
						if type( tags_text[ k ] ) == "table" then
							tag_i_ipol = tags_text[ k ][ 1 ] .. text_ipol( tags_text[ k ], i, #char_text, true )
						else
							tag_i_ipol = tags_text[ k ]
						end
						tag_n_ipol = tag_n_ipol .. tag_i_ipol
					end
					if char_text[ i ] == " " then
						return_text = return_text .. " "
					else
						return_text = return_text .. format( "{%s}%s", tag_n_ipol, char_text[ i ] )
					end
				end
				return return_text
			end, --!_G.ke4.text.tag( line.text_stripped, "\\fscy{ 100, 200, 50 }", { "\\1c", "&H00FFFF&", "&HFF00FF&", "&HFFFF00&" } )!
			
			inclip = function( Text_Config, Center, Middle )
				-- aplica fx solo al texto que esté dentro de un \clip
				if Text_Config.text:match( "\\x?clip%b()" ) then
					local shp_tbls = ke4.shape.divide( Text_Config.text:match( "\\x?clip%b()" ) )
					for i = 1, #shp_tbls do
						ke4.shape.info( shp_tbls[ i ] )
						if (Center >= minx and Center <= maxx)
							and (Middle >= miny and Middle <= maxy) then
							return true
						end
					end
				end
				return false
			end, --code syl: fxgroup.inclip = _G.ke4.text.inclip( line, line.left + syl.center, line.middle )
			
			outclip = function( Text_Config, Center, Middle )
				-- aplica fx solo al texto que esté dentro de un \clip
				if Text_Config.text:match( "\\x?clip%b()" ) then
					local shp_tbls = ke4.shape.divide( Text_Config.text:match( "\\x?clip%b()" ) )
					for i = 1, #shp_tbls do
						ke4.shape.info( shp_tbls[ i ] )
						if (Center >= minx and Center <= maxx)
							and (Middle >= miny and Middle <= maxy) then
							return false
						end
					end
				end
				return true
			end, --code syl: fxgroup.outclip = _G.ke4.text.outclip( line, line.left + syl.center, line.middle )
			
			romaji = {
				"a",	"i",	"u",	"e",	"o",	"ya",	"yu",	"yo",
				"ka",	"ki",	"ku",	"ke",	"ko",	"kya",	"kyu",	"kyo",
				"sa",	"shi",	"su",	"se",	"so",	"sha",	"shu",	"sho",
				"ta",	"chi",	"tsu",	"te",	"to",	"cha",	"chu",	"cho",
				"na",	"ni",	"nu",	"ne",	"no",	"nya",	"nyu",	"nyo",
				"ha",	"hi",	"fu",	"he",	"ho",	"hya",	"hyu",	"hyo",
				"ma",	"mi",	"mu",	"me",	"mo",	"mya",	"myu",	"myo",
				"ya",			"yu",			"yo",
				"ra",	"ri",	"ru",	"re",	"ro",	"rya",	"ryu",	"ryo",
				"wa",	"wi",			"we",	"wo",
				"n",
				"ga",	"gi",	"gu",	"ge",	"go",	"gya",	"gyu",	"gyo",
				"za",	"ji",	"zu",	"ze",	"zo",	"ja",	"ju",	"jo",
				"da",	"di",	"du",	"de",	"do",
				"ba",	"bi",	"bu",	"be",	"bo",	"bya",	"byu",	"byo",
				"pa",	"pi",	"pu",	"pe",	"po",	"pya",	"pyu",	"pyo",
				"b",	"c",	"d",	"k",	"p",	"r",	"s",	"t",
				"z"
			},
			
			hiragana = {
				"あ",	"い",	"う",	"え",	"お",	"ゃ",	"ゅ",	"ょ",
				"か",	"き",	"く",	"け",	"こ",	"きゃ",	"きゅ",	"きょ",
				"さ",	"し",	"す",	"せ",	"そ",	"しゃ",	"しゅ",	"しょ",
				"た",	"ち",	"つ",	"て",	"と",	"ちゃ",	"ちゅ",	"ちょ",
				"な",	"に",	"ぬ",	"ね",	"の",	"にゃ",	"にゅ",	"にょ",
				"は",	"ひ",	"ふ",	"へ",	"ほ",	"ひゃ",	"ひゅ",	"ひょ",
				"ま",	"み",	"む",	"め",	"も",	"みゃ",	"みゅ",	"みょ",
				"や",			"ゆ",			"よ",
				"ら",	"り",	"る",	"れ",	"ろ",	"りゃ",	"りゅ",	"りょ",
				"わ",	"ゐ",			"ゑ",	"を",
				"ん",
				"が",	"ぎ",	"ぐ",	"げ",	"ご",	"ぎゃ",	"ぎゅ",	"ぎょ",
				"ざ",	"じ",	"ず",	"ぜ",	"ぞ",	"じゃ",	"じゅ",	"じょ",
				"だ",	"ぢ",	"づ",	"で",	"ど",
				"ば",	"び",	"ぶ",	"べ",	"ぼ",	"びゃ",	"びゅ",	"びょ",
				"ぱ",	"ぴ",	"ぷ",	"ぺ",	"ぽ",	"ぴゃ",	"ぴゅ",	"ぴょ",
				"っ",	"っ",	"っ",	"っ",	"っ",	"っ",	"っ",	"っ",
				"っ"
			},
			
			katakana = {
				"ア",	"イ",	"ウ",	"エ",	"オ",	"ャ",	"ュ",	"ョ",
				"カ",	"キ",	"ク",	"ケ",	"コ",	"キャ",	"キュ",	"キョ",
				"サ",	"シ",	"ス",	"セ",	"ソ",	"シャ",	"シュ",	"ショ",
				"タ",	"チ",	"ッ",	"テ",	"ト",	"チャ",	"チュ",	"チョ",
				"ナ",	"ニ",	"ヌ",	"ネ",	"ノ",	"ニャ",	"ニュ",	"ニョ",
				"ハ",	"ヒ",	"フ",	"ヘ",	"ホ",	"ヒャ",	"ヒュ",	"ヒョ",
				"マ",	"ミ",	"ム",	"メ",	"モ",	"ミャ",	"ミュ",	"ミョ",
				"ヤ",			"ユ",			"ヨ",
				"ラ",	"リ",	"ル",	"レ",	"ロ",	"リャ",	"リュ",	"リョ",
				"ワ",	"ヰ",			"ヱ",	"ヲ",
				"ン",
				"ガ",	"ギ",	"グ",	"ゲ",	"ゴ",	"ギャ",	"ギュ",	"ギョ",
				"ザ",	"ジ",	"ズ",	"ゼ",	"ゾ",	"ジャ",	"ジュ",	"ジョ",
				"ダ",	"ヂ",	"ヅ",	"デ",	"ド",
				"バ",	"ビ",	"ブ",	"ベ",	"ボ",	"ビャ",	"ビュ",	"ビョ",
				"パ",	"ピ",	"プ",	"ペ",	"ポ",	"ピャ",	"ピュ",	"ピョ",
				"ッ",	"ッ",	"ッ",	"ッ",	"ッ",	"ッ",	"ッ",	"ッ",
				"ッ"
			},
			
			char_upper = {
				"A",	"B",	"C",	"D",	"E",	"F",	"G",	"H",
				"I",	"J",	"K",	"L",	"M",	"N",	"Ñ",	"O",
				"P",	"Q",	"R",	"S",	"T",	"U",	"V",	"W",
				"X",	"Y",	"Z"
			}, --string.char( R( 97, 122 ) )
			
			char_lower = {
				"a",	"b",	"c",	"d",	"e",	"f",	"g",	"h",
				"i",	"j",	"k",	"l",	"m",	"n",	"ñ",	"o",
				"p",	"q",	"r",	"s",	"t",	"u",	"v",	"w",
				"x",	"y",	"z"
			}, --string.char( R( 65, 90 ) )
			
			char_number = {
				"1",	"2",	"3",	"4",	"5",	"6",	"7",	"8",
				"9",	"0"
			}, --string.char( R( 48, 57 ) )
			
			char_special = {
				"°",	"¬",	"¡",	"!",	"¿",	"?",	"(",	")",
				"[",	"]",	"^",	"'",	"-",	"#",	"$",	"%",
				"&",	";",	":",	",",	".",	"<",	">",	"*",
				"~",	"´",	"`",	"¨",	"+",	"/",	"{",	"}",
				"|",	"_",	"\\",	"\""
			},
			
			karaoke_true = function( Table )
				local dur_ktime = { }
				for i = 1, #Table do
					dur_ktime[ i ] = { }
					for dur_k in Table[ i ]:gmatch( "\\[kK]^*[fo]*%d+" ) do
						table.insert( dur_ktime[ i ], dur_k:match( "%d+" ) )
					end
					if #dur_ktime[ i ] == 0 then
						return false
					end
				end
				return true
			end,
			
			remove_tags = function( Text )
				local  text_withouttags = Text:gsub( "%b{}", "" )
				return text_withouttags
			end, --!_G.ke4.text.remove_tags( line.text )!

			remove_space_in_tags = function( Text )
				local tags_raw, tags_KEx = { }, { }
				local tags_clp
				for c_rem in Text:gmatch( "%b{}" ) do
					table.insert( tags_raw, c_rem )
				end
				for i = 1, #tags_raw do
					tags_KEx[ i ] = tags_raw[ i ]
					tags_clp = { }
					for c_clp in tags_KEx[ i ]:gmatch( "\\i?clip%b()" ) do
						table.insert( tags_clp, c_clp )
					end
					tags_KEx[ i ] = tags_raw[ i ]:gsub( " ", "" )
					for k = 1, #tags_clp do
						tags_KEx[ i ] = tags_KEx[ i ]:gsub( "\\i?clip%b()", tags_clp[ k ]:gsub( " ", "KEclip" ), 1 )
					end
				end
				for i = 1, #tags_KEx do
					Text = Text:gsub( ke4.tag.operation( tags_raw[ i ] ), tags_KEx[ i ], 1 )
				end
				return Text
			end, --!_G.ke4.text.remove_space_in_tags( line.text )!
			
			remove_extra_space = function( Text )
				local linetext_chars = { }
				for c_spc in unicode.chars( Text ) do
					table.insert( linetext_chars, c_spc )
				end
				for i = 1, #linetext_chars do
					if linetext_chars[ 1 ] == " "
						or linetext_chars[ 1 ] == "	" then
						table.remove( linetext_chars, 1 )
					elseif linetext_chars[ #linetext_chars ] == " "
						or linetext_chars[ #linetext_chars ] == "	" then
						table.remove( linetext_chars, #linetext_chars )
					end
				end
				return ke4.table.op( linetext_chars, "concat" )
			end, --!_G.ke4.text.remove_extra_space( line.text )!
			
			remove_syls_nil = function( Text, Duration )	
				local syl_complete = ke4.text.text2syl( Text, Duration )
				local syl_tags, syl_texts, line_without_syl_nil = { }, { }, ""
				for i = 1, #syl_complete do
					syl_tags[ i ] = syl_complete[ i ]:match( "{%S+}" ) or ""
					syl_texts[ i ] = ke4.text.remove_tags( syl_complete[ i ] )
				end
				if syl_texts[ 1 ]
					and syl_texts[ 1 ]:gsub( " ", "" ):gsub( "	", "" ) == "" then
					syl_texts[ 1 ] = ""
				end
				if syl_texts[ #syl_texts ]
					and syl_texts[ #syl_texts - 1 ]
					and syl_texts[ #syl_texts ]:gsub( " ", "" ):gsub( "	", "" ) == ""
					and syl_texts[ #syl_texts - 1 ]:gsub( " ", "" ):gsub( "	", "" ) == "" then
					syl_texts[ #syl_texts - 1 ] = ""
				end
				for i = 1, #syl_complete do
					line_without_syl_nil = line_without_syl_nil .. syl_tags[ i ] .. syl_texts[ i ]
				end
				return line_without_syl_nil
			end,
			
			to_word = function( Text, Duration )
				local txt_str = Text:gsub( "\\N", " " )
				txt_str = ke4.text.remove_space_in_tags( txt_str )
				local Duration = Duration or 5000--fx.dur
				local words_in_line = { }
				for wrd in txt_str:gmatch( "[{.-}]*[%S+]*[\33-\47\58-\64]*[%s]*" ) do
					table.insert( words_in_line, wrd )
				end
				table.remove( words_in_line, #words_in_line )
				if words_in_line[ 1 ] then
					local word_in_t_1 = ke4.text.remove_tags( words_in_line[ 1 ] ):gsub( " ", "" ):gsub( "	", "" )
					if unicode.len( word_in_t_1 ) == 0 then
						words_in_line[ 1 ] = words_in_line[ 1 ]:gsub( " ", "" ):gsub( "	", "" ) .. "KEfx"
					end
					if words_in_line[ 1 ] == "KEfx" then
						table.remove( words_in_line, 1 )
					end
				end
				if #words_in_line > 0 then
					words_in_line[ #words_in_line ] = words_in_line[ #words_in_line ]:gsub( " ", "" ):gsub( "	", "" )
				end
				if #words_in_line == 0 then
					words_in_line[ 1 ] = format( "{\\k%d}", Duration / 10 )
				end
				return words_in_line
			end, --!_G.ke4.table.view( _G.ke4.text.to_word( line.text, line.duration ) )!

			text2word = function( Text, Duration )
				local Duration = Duration or 5000--fx.dur
				local words_in_text = ke4.text.to_word( Text, Duration )
				local words_in_text_dur, count_chars_in_line = { }
				local text_without_space = ke4.text.remove_tags( Text ):gsub( " ", "" )
				local word_without_space
				if ke4.text.karaoke_true( words_in_text ) == true then
					for i = 1, #words_in_text do
						words_in_text_dur[ i ] = { }
						for tag_k in words_in_text[ i ]:gmatch( "\\[kK]^*[fo]*%d+" ) do
							table.insert( words_in_text_dur[ i ], tag_k:match( "%d+" ) )
						end
						words_in_text_dur[ i ] = ke4.table.op( words_in_text_dur[ i ], "sum" ) * 10
					end
				else
					count_chars_in_line = unicode.len( text_without_space )
					for i = 1, #words_in_text do
						word_without_space = ke4.text.remove_tags( words_in_text[ i ] ):gsub( " ", "" )
						words_in_text_dur[ i ] = ke4.math.round(
							unicode.len( word_without_space ) * Duration / count_chars_in_line, 3
						)
					end
				end
				return words_in_text, words_in_text_dur
			end, --!_G.ke4.table.view( _G.ke4.text.text2word( line.text, line.duration ) )!

			text2syl = function( Text, Duration )
				local words_in_text = ke4.text.to_word( Text, Duration )
				local syls_in_text, syls_in_text_dur, chars_in_text = { }, { }
				local text_without_space = ke4.text.remove_tags( Text ):gsub( " ", "" )
				local word_without_space
				if ke4.text.karaoke_true( words_in_text ) == true then
					for i = 1, #words_in_text do
						for syls in words_in_text[ i ]:gmatch( "{.-}[\32-\122\124\126-\255]*" ) do
							table.insert( syls_in_text, syls )
						end
					end
					for i = 1, #syls_in_text do
						if ke4.text.remove_tags( syls_in_text[ i ] ) == "" then
							syls_in_text[ i ] = syls_in_text[ i ] .. "KEfx"
						end
					end
					for i = 1, #syls_in_text do
						syls_in_text_dur[ i ] = { }
						for tagk in syls_in_text[ i ]:gmatch( "\\[kK]^*[fo]*%d+" ) do
							table.insert( syls_in_text_dur[ i ], tagk:match( "%d+" ) )
						end
						syls_in_text_dur[ i ] = ke4.table.op( syls_in_text_dur[ i ], "sum" ) * 10
					end
				else
					syls_in_text = words_in_text
					chars_in_text = unicode.len( text_without_space )
					for i = 1, #words_in_text do
						word_without_space = ke4.text.remove_tags( words_in_text[ i ] ):gsub( " ", "" )
						syls_in_text_dur[ i ] = ke4.math.round(
							unicode.len( word_without_space ) * Duration / chars_in_text, 3
						)
					end
				end
				return syls_in_text, syls_in_text_dur
			end, --!_G.ke4.table.view( _G.ke4.text.text2syl( line.text, line.duration ) )!

			text2char = function( Text, Duration )
				local words_in_text = ke4.text.to_word( Text, Duration )
				local syls_in_text, syls_in_text_stp, syls_in_text_dur = { }, { }, { }
				local chars_in_text, chars_in_text_dur, chars_in_linetext_str = { }, { }
				if ke4.text.karaoke_true( words_in_text ) == true then
					for i = 1, #words_in_text do
						for c_2c1 in words_in_text[ i ]:gmatch( "{.-}[\32-\122\124\126-\255]*" ) do
							table.insert( syls_in_text, c_2c1 )
						end
					end
					for i = 1, #syls_in_text do
						if ke4.text.remove_tags( syls_in_text[ i ] ) == "" then
							syls_in_text[ i ] = syls_in_text[ i ] .. "KEfx"
						end
					end
					for i = 1, #syls_in_text do
						syls_in_text_dur[ i ] = { }
						for c_2c2 in syls_in_text[ i ]:gmatch( "\\[kK]^*[fo]*%d+" ) do
							table.insert( syls_in_text_dur[ i ], c_2c2:match( "%d+" ) )
						end
						syls_in_text_dur[ i ] = ke4.table.op( syls_in_text_dur[ i ], "sum" ) * 10
					end
					for i = 1, #syls_in_text do
						syls_in_text_stp[ i ] = ke4.text.remove_tags( syls_in_text[ i ] )
						if syls_in_text_stp[ i ] == "KEfx" then
							table.insert( chars_in_text, "KEfx" )
						else
							for c_2c3 in unicode.chars( syls_in_text_stp[ i ] ) do
								table.insert( chars_in_text, c_2c3 )
							end
						end
						if syls_in_text_stp[ i ] == "KEfx" then
							table.insert( chars_in_text_dur, syls_in_text_dur[ i ] )
						else
							if syls_in_text_stp[ i ]:gsub( " ", "" ) == ""
								or syls_in_text_stp[ i ]:gsub( " ", "" ) == syls_in_text_stp[ i ] then
								for k = 1, unicode.len( syls_in_text_stp[ i ] ) do
									table.insert(
										chars_in_text_dur, ke4.math.round( syls_in_text_dur[ i ] / unicode.len( syls_in_text_stp[ i ] ), 2 )
									)
								end
							else
								local syl_in_text_stp2
								for k = 1, unicode.len( syls_in_text_stp[ i ] ) do
									if ke4.table.string( syls_in_text_stp[ i ] )[ k ] == " " then
										table.insert( chars_in_text_dur, 0 )
									else
										syl_in_text_stp2 = syls_in_text_stp[ i ]:gsub( " ", "" )
										table.insert( chars_in_text_dur,
											ke4.math.round( syls_in_text_dur[ i ] / unicode.len( syl_in_text_stp2 ), 2 )
										)
									end
								end
							end
						end
					end
				else
					for c_2c4 in unicode.chars( ke4.text.remove_tags( Text ) ) do
						table.insert( chars_in_text, c_2c4 )
					end
					chars_in_linetext_str = #ke4.table.retire( ke4.table.duplicate( chars_in_text ), " " )
					for i = 1, #chars_in_text do
						if chars_in_text[ i ] == " "
							or chars_in_text[ i ] == "" then
							table.insert( chars_in_text_dur, 0 )
						else
							table.insert( chars_in_text_dur, ke4.math.round( Duration / chars_in_linetext_str, 3 ) )
						end
					end
				end
				return chars_in_text, chars_in_text_dur
			end,
			
			text2part = function( Text_Config, Text, Duration, Text_left, Parts )
				local function text_width( Text_Config, String )
					local txt_width = aegisub.text_extents( Text_Config, String )
					return txt_width
				end
				local Text = Text or "_G.ke4.trxt.text2part"
				local Duration = Duration or 5000
				local Text_left = Text_left or 0
				local Parts = Parts or 2
				local parts_in_text = ke4.string.parts( Text, Parts )
				local parts_in_text_dur = { }
				local parts_widths, parts_lefts = { [ 0 ] = 0 }, { [ 0 ] = Text_left }
				local parts_rights, parts_centers = { }, { }
				local Char_dur = ke4.math.round( Duration / unicode.len( Text ), 3 )
				local left_spc
				for i = 1, #parts_in_text do
					left_spc = ""
					parts_in_text_dur[ i ] = unicode.len( parts_in_text[ i ] ) * Char_dur
					while parts_in_text[ i ]:sub( 1, 1 ) == " "
						or parts_in_text[ i ]:sub( 1, 1 ) == "	" do
						left_spc = left_spc .. parts_in_text[ i ]:sub( 1, 1 )
						parts_in_text[ i ] = parts_in_text[ i ]:sub( 2, -1 )
					end
					parts_widths[ i ] = text_width( Text_Config, parts_in_text[ i ] )
					parts_lefts[ i ] = ke4.math.round( parts_lefts[ i - 1 ] + parts_widths[ i - 1 ] + text_width( Text_Config, left_spc ), 3 )
				end
				for i = 1, #parts_in_text do
					while parts_in_text[ i ]:sub( -1, -1 ) == " "
						or parts_in_text[ i ]:sub( -1, -1 ) == "	" do
						parts_in_text[ i ] = parts_in_text[ i ]:sub( 1, -2 )
					end
					parts_widths[ i ]  = ke4.math.round( text_width( Text_Config, parts_in_text[ i ] ), 3 )
					parts_rights[ i ]  = ke4.math.round( parts_lefts[ i ] + parts_widths[ i ], 3 )
					parts_centers[ i ] = ke4.math.round( parts_lefts[ i ] + parts_widths[ i ] / 2, 3 )
				end --!_G.ke4.table.view( _G.ke4.text.text2part( line.styleref, line.text_stripped, line.duration, line.left, { 2, 4 } ) )!
				parts_widths[ 0 ] = nil
				parts_lefts[ 0 ] = nil
				return parts_in_text, parts_in_text_dur, parts_centers, parts_widths, parts_lefts, parts_rights
			end, --local p_txt, p_dur, p_cen, p_wid, p_lef, p_rig
			
			syl2hiragana = function( Text )
				local idx
				if ke4.table.inside( ke4.text.romaji, Text:lower( ):match( "%w+" ) ) == true then
					idx = ke4.table.index( ke4.text.romaji, Text:lower( ):match( "%w+" ) )
					return Text:lower( ):gsub( ke4.text.romaji[ idx ], ke4.text.hiragana[ idx ] )
				elseif ke4.table.inside( ke4.text.katakana, Text:match( "[\128-\255]+" ) ) == true then
					idx = ke4.table.index( ke4.text.katakana, Text:match( "[\128-\255]+" ) )
					return Text:gsub( ke4.text.katakana[ idx ], ke4.text.hiragana[ idx ] )
				end
				return Text
			end,
			
			syl2katakana = function( Text )
				local idx
				if ke4.table.inside( ke4.text.romaji, Text:lower( ):match( "%w+" ) ) == true then
					idx = ke4.table.index( ke4.text.romaji, Text:lower( ):match( "%w+" ) )
					return Text:lower( ):gsub( ke4.text.romaji[ idx ], ke4.text.katakana[ idx ] )
				elseif ke4.table.inside( ke4.text.hiragana, Text:match( "[\128-\255]+" ) ) == true then
					idx = ke4.table.index( ke4.text.hiragana, Text:match( "[\128-\255]+" ) )
					return Text:gsub( ke4.text.hiragana[ idx ], ke4.text.katakana[ idx ] )
				end
				return Text
			end, --!_G.ke4.text.syl2katakana( syl.text )!
			
			kana2romaji = function( Text )
				local idx
				if ke4.table.inside( ke4.text.hiragana, Text:match( "[\128-\255]+" ) ) == true then
					idx = ke4.table.index( ke4.text.hiragana, Text:match( "[\128-\255]+" ) )
					return Text:gsub( ke4.text.hiragana[ idx ], ke4.text.romaji[ idx ] )
				elseif ke4.table.inside( ke4.text.katakana, Text:match( "[\128-\255]+" ) ) == true then
					idx = ke4.table.index( ke4.text.katakana, Text:match( "[\128-\255]+" ) )
					return Text:gsub( ke4.text.katakana[ idx ], ke4.text.romaji[ idx ] )
				end
				return Text
			end,
			
			text2stripped = function( Text )
				local Text = ke4.text.remove_tags( Text )
				local text_without_space = Text:gsub( " ", "" )
				if unicode.len( text_without_space ) == 0 then
					return Text
				end
				return Text:gsub( " ", "" )
			end,

			char2byte = function( Text )
				local bytes = { }
				for c in unicode.chars( Text ) do
					table.insert( bytes, c:byte( ) )
				end
				return bytes
			end,
			
			byte2char = function( Bytes )
				return string.char( unpack( Bytes ) )
			end,
			
		},
		
		-- Shape sublibrary
		shape = {
			-- shapes prediseñadas internas --------------------
			circle		= "m 50 0 b 22 0 0 22 0 50 b 0 78 22 100 50 100 b 78 100 100 78 100 50 b 100 22 78 0 50 0 ",
			triangle	= "m 50 0 l 0 86 l 100 86 l 50 0 ",
			rectangle	= "m 0 0 l 0 100 l 100 100 l 100 0 l 0 0 ",
			circangle	= "m 20 0 b 8 0 0 8 0 20 l 0 80 b 0 92 8 100 20 100 l 80 100 b 92 100 100 92 100 80 l 100 20 b 100 8 92 0 80 0 l 20 0 ",
			pixel		= "m 0 0 l 0 1 l 1 1 l 1 0 l 0 0 ",
			pentagon	= "m 50 0 l 0 36 l 19 95 l 81 95 l 100 36 l 50 0 ",
			hexagon		= "m 50 0 l 0 29 l 0 87 l 50 116 l 100 87 l 100 29 l 50 0 ",
			octagon		= "m 29 0 l 0 29 l 0 71 l 29 100 l 71 100 l 100 71 l 100 29 l 71 0 l 29 0 ",
			heart		= "m 50 25 b 32 0 0 16 0 40 b 0 68 24 71 50 106 b 75 71 100 68 100 40 b 100 16 68 0 50 25 ",
			heart2t		= "m 50 25 b 32 0 0 16 0 40 b 0 68 24 71 50 100 b 75 71 100 68 100 40 b 100 16 68 0 50 25 m 79 27 b 74 25 76 20 81 22 b 90 26 93 36 93 43 b 93 48 87 48 87 43 b 87 39 86 30 79 27 ",
			heart_b		= "m 50 25 b 32 0 0 16 0 40 b 0 68 24 71 50 100 b 75 71 100 68 100 40 b 100 16 68 0 50 25 l 50 28 b 69 2 98 18 98 40 b 98 66 74 69 50 97 b 25 69 2 66 2 40 b 2 18 31 2 50 28 ",
			shine1t		= "m 0 0 l 47 50 l 0 100 l 50 53 l 100 100 l 53 50 l 100 0 l 50 47 m 42 50 b 42 61 58 61 58 50 b 58 39 42 39 42 50 ",
			shine2t		= "m 0 0 l 45 50 l 0 100 l 50 55 l 100 100 l 55 50 l 100 0 l 50 45 m 40 50 b 40 64 60 64 60 50 b 60 36 40 36 40 50 ",
			shine3t		= "m 0 0 b 35 39 61 65 100 100 b 65 61 39 35 0 0 m 100 0 b 61 35 35 61 0 100 b 39 65 65 39 100 0 ",
			shine4t		= "m 50 100 b 52 69 52 31 50 0 b 48 31 48 69 50 100 m 0 50 b 31 52 69 52 100 50 b 69 48 31 48 0 50 ",
			trebol		= "m 1 99 l 4 106 b 21 99 44 83 53 56 b 51 73 40 88 56 98 b 72 106 80 90 77 82 b 85 86 100 82 100 69 b 100 58 87 52 64 54 b 52 55 51 50 68 48 b 85 46 94 40 95 29 b 96 18 80 10 70 19 b 70 0 40 0 40 21 b 40 33 47 37 50 43 b 54 50 52 54 47 47 b 39 38 31 26 19 27 b 0 29 0 49 13 53 b 0 58 3 80 19 79 b 39 78 40 62 51 55 b 42 79 23 92 1 99 ",
			feather		= "m 0 0 b 0 20 10 28 27 34 b 10 33 47 49 54 55 b 62 62 72 77 78 75 l 80 78 l 90 79 b 94 86 96 94 97 100 l 100 100 b 99 93 96 84 93 78 b 100 56 88 41 73 30 l 73 39 l 69 28 b 62 24 55 23 49 19 l 48 24 l 45 19 b 31 10 13 12 0 0 m 91 74 l 88 75 b 79 49 57 40 46 35 b 22 25 8 15 2 5 b 11 17 22 23 48 34 b 64 41 82 51 91 74 ",
			feather2	= "m 99 8 l 87 18 b 83 13 75 18 75 23 b 69 19 68 29 62 27 b 53 27 50 21 44 21 b 38 15 31 11 21 8 b 14 2 7 0 4 2 b 0 16 11 37 29 43 b 40 49 65 50 78 31 b 83 34 93 29 89 20 l 100 9 ",
			diamond		= "m 50 0 l 0 50 l 50 100 l 100 50 l 50 0 ",
			gear		= "m 3 70 l 3 70 l 17 69 l 25 78 l 22 92 l 30 97 l 40 86 l 53 86 l 60 99 l 70 96 l 68 82 l 79 73 l 92 78 l 97 69 l 86 60 l 86 46 l 100 38 l 96 29 l 82 31 l 74 21 l 77 7 l 70 3 l 60 14 l 46 13 l 38 0 l 30 2 l 30 18 l 20 26 l 6 23 l 1 31 l 13 41 l 12 53 l 0 61 l 3 70 m 24 49 b 24 15 74 15 74 49 b 74 83 24 83 24 49 ",
			bubble		= "m 50 100 b 78 100 100 78 100 50 b 100 22 78 0 50 0 b 22 0 0 22 0 50 b 0 78 22 100 50 100 m 50 1 b 79 1 99 21 99 50 b 99 76 80 93 68 96 b 62 98 66 94 50 94 b 34 94 38 98 32 96 b 20 93 1 78 1 50 b 1 22 22 1 50 1 m 88 22 b 79 11 75 14 85 24 b 92 38 94 33 88 22 m 12 15 b 12 19 18 19 18 15 b 18 11 12 11 12 15 m 14 16 l 15 30 l 16 16 l 30 15 l 16 14 l 15 0 l 14 14 l 0 15 m 50 94 b 63 94 61 100 52 100 b 42 100 38 94 50 94 ",
			note1t		= "m 33 70 b 16 57 0 67 0 82 b 0 93 9 100 19 100 b 28 100 40 93 40 73 l 40 15 b 46 18 53 25 56 36 b 56 18 48 8 33 0 l 33 70 ",
			note2t		= "m 33 70 b 16 57 0 67 0 82 b 0 93 9 100 19 100 b 28 100 40 93 40 73 l 40 27 l 93 18 l 93 55 b 76 42 60 52 60 67 b 60 78 69 85 79 85 b 88 85 100 78 100 58 l 100 0 l 33 12 l 33 70 ",
			note3t		= "m 33 70 b 16 57 0 67 0 82 b 0 93 9 100 19 100 b 28 100 40 93 40 73 l 40 36 l 93 27 l 93 55 b 76 42 60 52 60 67 b 60 78 69 85 79 85 b 88 85 100 78 100 58 l 100 0 l 33 12 l 33 70 m 40 29 l 40 18 l 93 8 l 93 20 l 40 29 ",
			note4t		= "m 33 70 b 16 57 0 67 0 82 b 0 93 9 100 19 100 b 28 100 40 93 40 73 l 40 28 l 93 18 l 93 55 b 76 42 60 52 60 67 b 60 78 69 85 79 85 b 88 85 100 78 100 58 l 100 0 l 33 12 l 33 70 m 35 73 l 35 13 l 98 2 l 98 59 b 98 77 87 83 79 83 b 70 83 62 77 62 67 b 62 53 76 44 95 58 l 95 16 l 38 27 l 38 74 b 38 91 27 98 19 98 b 10 98 2 91 2 82 b 2 68 16 59 35 73 ",
			star		= "m 38 36 l 0 36 l 31 59 l 20 95 l 50 72 l 81 94 l 69 59 l 100 36 l 62 36 l 50 0 l 38 36 ",
			star1t		= "m 0 35 l 29 56 l 33 55 b 37 44 42 66 34 58 l 30 60 l 18 95 l 48 73 l 48 69 b 38 63 60 63 51 69 l 51 73 l 81 95 l 70 60 l 65 58 b 57 65 63 44 66 55 l 71 56 l 100 35 l 63 35 l 61 39 b 64 48 47 36 59 36 l 61 33 l 50 0 l 38 33 l 40 36 b 52 36 36 49 38 39 l 36 35 l 0 35 ",
			star2t		= "m 31 26 b 0 29 0 29 14 58 b 0 87 0 87 31 89 b 50 116 50 116 69 89 b 100 87 100 87 86 58 b 100 29 100 29 69 26 b 50 0 50 0 31 26 l 33 29 b 50 16 50 16 67 29 b 86 37 86 37 83 58 b 86 79 86 79 67 87 b 50 100 50 100 33 87 b 14 79 14 79 17 58 b 14 37 14 37 33 29 ",
			star3t		= "m 31 26 b 0 29 0 29 14 58 b 0 87 0 87 31 89 b 50 116 50 116 69 89 b 100 87 100 87 86 58 b 100 29 100 29 69 26 b 50 0 50 0 31 26 ",
			star4t		= "m 25 72 l 18 61 l 0 87 l 33 87 b 42 72 53 53 25 72 m 50 87 l 37 87 l 50 116 l 67 87 b 58 72 47 53 50 87 m 75 72 l 69 83 l 100 87 l 83 58 b 66 58 45 58 75 72 m 75 43 l 82 54 l 100 29 l 67 29 b 58 45 47 63 75 43 m 50 29 l 63 29 l 50 0 l 33 29 b 42 45 53 63 50 29 m 25 43 l 31 32 l 0 29 l 16 58 b 34 58 54 58 25 43 ",
			star5t		= "m 33 29 l 0 29 l 50 116 l 100 29 l 33 29 l 50 0 l 100 87 l 0 87 l 33 29 ",
			star6t		= "m 36 33 b 0 36 0 36 27 60 b 20 95 20 95 50 77 b 81 95 81 95 73 60 b 100 36 100 36 64 33 b 50 0 50 0 36 33 m 61 35 b 95 38 95 38 69 59 b 78 90 78 90 50 73 b 23 90 23 90 31 59 b 6 38 6 38 39 35 b 50 6 50 6 61 35 ",
			star7t		= "m 36 33 b 0 36 0 36 27 60 b 20 95 20 95 50 77 b 81 95 81 95 73 60 b 100 36 100 36 64 33 b 50 0 50 0 36 33 ",
			star8t		= "m 39 36 b 0 36 0 36 31 59 b 20 95 20 95 50 72 b 81 95 81 95 69 59 b 100 36 100 36 62 36 b 50 0 50 0 39 36 ",
			star9t		= "m 0 29 l 16 58 l 0 87 l 33 87 l 50 116 l 67 87 l 100 87 l 84 58 l 100 29 l 67 29 l 50 0 l 33 29 l 0 29 l 10 35 l 37 35 l 50 12 l 63 35 l 90 35 l 77 58 l 90 81 l 63 81 l 50 104 l 37 81 l 10 81 l 23 58 l 10 35 ",
			star10t 	= "m 0 29 l 16 58 l 0 87 l 33 87 l 50 116 l 67 87 l 100 87 l 84 58 l 100 29 l 67 29 l 50 0 l 33 29 l 0 29 ",
			sakura		= "m 50 40 l 35 0 b 10 10 0 32 0 61 b 0 88 15 117 50 130 b 85 117 100 88 100 61 b 100 32 90 10 65 0 l 50 40 ",
			sakura1t 	= "m 41 57 b 23 57 1 53 0 82 b 26 97 34 76 44 60 l 11 76 l 41 57 m 47 62 b 38 78 23 95 50 110 b 76 95 62 78 52 62 l 50 99 l 47 62 m 56 60 b 66 76 73 97 100 82 b 99 53 77 57 58 57 l 89 76 l 56 60 m 44 50 b 34 33 26 13 0 27 b 1 57 23 53 41 53 l 11 33 l 44 50 m 52 47 b 62 32 76 15 50 0 b 23 15 38 32 47 47 l 50 12 l 52 47 m 58 53 b 77 53 99 57 100 27 b 73 13 66 33 56 50 l 89 33 l 58 53 ",
			sakura2t 	= "m 50 52 l 50 40 l 16 20 l 0 28 l 0 46 l 15 55 l 27 46 l 14 36 l 50 52 m 46 54 l 35 48 l 0 67 l 0 86 l 16 94 l 30 86 l 30 72 l 14 79 l 46 54 m 46 59 l 34 67 l 34 104 l 50 114 l 66 104 l 66 88 l 52 81 l 50 100 l 46 59 m 50 62 l 50 75 l 84 94 l 100 86 l 100 67 l 85 60 l 73 67 l 87 79 l 50 62 m 54 60 l 66 65 l 100 46 l 100 28 l 84 20 l 69 28 l 70 44 l 87 36 l 54 60 m 54 55 l 66 47 l 66 10 l 50 0 l 34 10 l 34 26 l 48 34 l 50 14 l 54 55 ",
			sakura3t 	= "m 47 49 b 25 49 10 52 0 63 l 0 78 l 12 85 b 21 85 26 80 31 75 b 33 69 36 63 39 58 l 18 67 l 47 49 m 44 55 b 35 70 30 84 32 102 l 47 110 l 60 102 b 61 92 61 85 59 80 b 54 76 50 70 48 65 l 47 89 l 44 55 m 48 58 b 55 73 66 88 82 92 l 95 84 l 95 69 b 90 64 85 62 78 60 b 73 61 66 62 60 61 l 79 75 l 48 58 m 53 57 b 71 60 91 54 100 46 l 100 31 l 86 22 b 79 24 72 28 68 33 b 67 39 65 45 61 50 l 80 40 l 53 57 m 56 53 b 65 39 69 24 68 8 l 53 0 l 39 8 b 38 15 38 21 41 27 b 45 31 49 36 52 41 l 53 20 l 56 53 m 52 49 b 47 34 31 21 18 15 l 4 24 l 4 39 b 9 44 15 47 23 48 b 29 46 35 46 41 46 l 22 33 l 52 49 ",
			sakura4t 	= "m 42 59 l 18 59 b 0 59 0 74 0 86 l 25 86 b 36 86 40 70 47 62 l 25 73 l 42 59 m 48 65 l 35 86 b 26 101 38 107 50 114 l 63 93 b 69 85 57 71 53 62 l 51 87 l 48 65 m 57 62 l 69 85 b 77 100 89 92 100 85 l 89 64 b 83 55 67 57 56 57 l 78 71 l 57 62 m 58 55 l 84 55 b 100 55 100 40 100 29 l 76 29 b 65 29 61 43 53 53 l 76 42 l 58 55 m 52 49 l 64 30 b 75 14 62 7 50 0 l 38 18 b 32 29 43 40 47 53 l 50 27 l 52 49 m 44 53 l 33 30 b 24 15 10 23 0 30 l 12 50 b 19 59 35 57 44 57 l 25 43 l 44 53 ",
			sakura5t 	= "m 43 55 b 33 49 4 55 4 65 b 4 76 14 90 24 90 b 33 90 27 74 43 61 l 23 68 l 43 55 m 49 61 b 38 65 27 93 35 99 b 44 105 61 104 66 96 b 72 89 55 85 54 64 l 49 85 l 49 61 m 55 59 b 55 70 75 93 85 88 b 93 83 100 66 96 59 b 91 50 80 63 61 56 l 77 69 l 55 59 m 58 52 b 68 58 98 52 98 41 b 98 31 87 18 77 18 b 68 18 74 33 58 46 l 78 39 l 58 52 m 52 46 b 63 41 74 12 66 6 b 57 0 39 2 33 10 b 29 18 45 22 48 43 l 52 21 l 52 46 m 45 46 b 46 36 25 12 15 18 b 6 24 0 41 5 49 b 10 58 20 43 40 50 l 23 38 l 45 46 ",
			sakura6t	= "m 46 75 b 29 79 37 99 50 110 b 64 99 72 79 53 75 l 50 90 l 46 75 m 30 61 b 19 48 4 65 0 81 b 15 87 39 85 33 68 l 18 71 l 30 61 m 67 68 b 61 85 85 87 100 81 b 96 65 81 48 70 61 l 82 71 l 67 68 m 53 34 b 72 31 64 10 50 0 b 37 10 29 32 46 34 l 50 21 l 53 34 m 69 48 b 79 60 96 43 100 28 b 85 22 61 25 67 41 l 82 38 l 69 48 m 34 41 b 39 25 15 22 0 28 b 3 43 22 60 31 48 l 18 38 l 34 41 ",
			sakura7t 	= "m 46 75 b 28 79 37 99 50 110 b 64 99 72 79 53 75 l 50 90 l 46 75 m 30 61 b 19 48 4 65 0 81 b 15 87 39 85 33 68 l 18 71 l 30 61 m 66 68 b 61 85 85 87 100 81 b 96 65 81 48 69 61 l 82 71 l 66 68 m 53 34 b 72 31 64 10 50 0 b 37 10 28 31 46 34 l 50 21 l 53 34 m 69 48 b 81 60 96 43 100 28 b 85 22 61 25 66 41 l 82 38 l 69 48 m 33 41 b 39 25 15 22 0 28 b 4 43 19 60 30 48 l 18 38 l 33 41 m 55 42 b 55 34 44 34 44 42 b 44 50 55 50 55 42 m 55 67 b 55 59 44 59 44 67 b 44 75 55 75 55 67 m 44 48 b 44 40 33 40 33 48 b 33 56 44 56 44 48 m 44 61 b 44 53 33 53 33 61 b 33 69 44 69 44 61 m 66 48 b 66 40 55 40 55 48 b 55 56 66 56 66 48 m 66 61 b 66 53 55 53 55 61 b 55 69 66 69 66 61 ",
			snow1t		= "m 30 45 l 30 52 l 20 52 l 20 55 l 30 55 l 30 62 l 16 70 l 2 61 l 0 64 l 13 72 l 1 79 l 3 82 l 15 75 l 15 90 l 18 90 l 18 73 l 32 65 l 39 69 l 33 78 l 36 80 l 42 71 l 48 74 l 48 89 l 32 98 l 34 101 l 48 93 l 48 108 l 52 108 l 52 93 l 66 101 l 67 98 l 52 89 l 52 74 l 58 71 l 64 80 l 67 78 l 61 69 l 67 65 l 82 73 l 82 90 l 85 90 l 85 75 l 97 82 l 99 79 l 87 72 l 100 64 l 98 61 l 84 70 l 70 62 l 70 55 l 80 55 l 80 52 l 70 52 l 70 45 l 84 37 l 98 46 l 100 43 l 87 35 l 99 28 l 97 25 l 85 32 l 85 16 l 82 17 l 82 34 l 68 42 l 61 38 l 67 30 l 64 28 l 58 36 l 52 33 l 52 18 l 68 9 l 66 6 l 52 14 l 52 0 l 48 0 l 48 14 l 34 6 l 32 9 l 48 18 l 48 33 l 42 36 l 36 28 l 33 30 l 39 38 l 32 42 l 18 34 l 18 17 l 15 16 l 15 32 l 3 25 l 1 28 l 13 35 l 0 43 l 2 46 l 16 37 l 30 45 l 35 45 l 50 37 l 65 45 l 65 62 l 50 70 l 35 62 l 35 45 ",
			snow2t		= "m 18 45 l 14 29 l 6 31 l 8 39 l 0 41 l 2 49 l 18 45 l 21 44 l 32 42 l 42 47 l 32 53 l 21 51 l 19 59 l 27 60 l 23 62 l 8 59 l 5 66 l 13 68 l 11 76 l 19 77 l 23 62 l 27 60 l 25 68 l 33 70 l 35 60 l 46 54 l 46 65 l 38 72 l 35 75 l 22 87 l 28 93 l 35 87 l 42 93 l 48 87 l 35 75 l 38 72 l 45 78 l 50 73 l 56 78 l 62 72 l 65 75 l 52 87 l 59 93 l 65 87 l 71 93 l 78 86 l 65 75 l 62 72 l 54 65 l 54 54 l 65 60 l 67 70 l 75 68 l 73 60 l 77 62 l 80 77 l 89 76 l 87 68 l 95 67 l 93 59 l 77 62 l 73 60 l 80 59 l 78 51 l 68 53 l 58 47 l 68 42 l 78 44 l 81 45 l 97 49 l 100 41 l 92 39 l 94 31 l 86 29 l 81 45 l 78 44 l 80 37 l 73 35 l 75 26 l 67 24 l 67 21 l 84 26 l 86 18 l 77 15 l 79 7 l 71 5 l 67 21 l 67 24 l 65 35 l 54 40 l 54 29 l 63 22 l 57 16 l 50 22 l 50 17 l 62 6 l 56 0 l 50 6 l 44 0 l 38 6 l 50 17 l 50 22 l 43 16 l 37 22 l 46 29 l 46 40 l 35 35 l 33 24 l 33 21 l 29 5 l 21 7 l 23 15 l 14 18 l 16 26 l 33 21 l 33 24 l 25 26 l 27 35 l 19 37 l 21 44 ",
			snow3t		= "m 25 39 l 11 35 l 0 47 l 11 59 l 25 55 l 19 64 l 17 61 l 6 66 l 14 68 l 11 75 l 22 69 l 20 66 l 32 65 l 22 74 l 24 90 l 39 86 l 44 74 l 49 83 l 44 83 l 44 94 l 50 87 l 56 94 l 56 83 l 51 83 l 56 74 l 61 86 l 76 90 l 78 74 l 68 65 l 80 66 l 78 69 l 88 75 l 86 68 l 94 67 l 83 61 l 81 64 l 75 55 l 88 59 l 100 47 l 88 35 l 75 39 l 81 30 l 83 33 l 94 27 l 86 26 l 88 18 l 78 24 l 80 28 l 68 29 l 78 19 l 76 4 l 61 8 l 56 20 l 51 11 l 56 11 l 56 0 l 50 7 l 44 0 l 44 11 l 49 11 l 43 20 l 39 8 l 24 4 l 22 19 l 32 29 l 20 28 l 22 24 l 11 18 l 14 26 l 6 27 l 17 33 l 19 30 l 25 39 l 46 47 b 38 47 29 57 19 47 l 28 41 l 46 47 l 48 43 b 43 36 29 35 34 19 l 42 24 l 48 43 l 52 43 l 58 24 l 66 19 b 71 35 56 36 52 43 l 54 47 l 72 41 l 81 46 b 71 57 62 47 54 47 l 52 51 b 57 58 71 59 66 75 l 58 70 l 52 51 l 48 51 l 42 70 l 34 75 b 29 59 42 58 48 51 l 46 47 ",
			flower1t 	= "m 44 44 b 30 41 25 18 0 36 b 10 66 22 68 40 56 b 33 69 10 66 20 95 b 50 95 56 84 50 62 b 61 75 50 95 81 95 b 90 66 81 56 60 56 b 76 50 90 66 100 36 b 75 18 65 22 56 44 b 57 26 75 18 50 0 b 25 18 28 33 44 44 ",
			flower2t 	= "m 44 44 b 30 41 25 18 0 36 b 10 66 22 68 40 56 b 33 69 10 66 20 95 b 50 95 56 84 50 62 b 61 75 50 95 81 95 b 90 66 81 56 60 56 b 76 50 90 66 100 36 b 75 18 65 22 56 44 b 57 26 75 18 50 0 b 25 18 28 33 44 44 m 43 15 b 43 6 55 6 55 15 b 55 24 43 24 43 15 m 9 42 b 9 33 21 33 21 42 b 21 51 9 51 9 42 m 23 82 b 23 73 35 73 35 82 b 35 91 23 91 23 82 m 66 82 b 66 73 78 73 78 82 b 78 91 66 91 66 82 m 78 40 b 78 31 90 31 90 40 b 90 49 78 49 78 40 ",
			flower3t 	= "m 44 44 b 30 41 25 18 0 36 b 10 66 22 68 40 56 b 33 69 10 66 20 95 b 50 95 56 84 50 62 b 61 75 50 95 81 95 b 90 66 81 56 60 56 b 76 50 90 66 100 36 b 75 18 65 22 56 44 b 57 26 75 18 50 0 b 25 18 28 33 44 44 m 19 23 b 19 14 31 14 31 23 b 31 32 19 32 19 23 m 8 69 b 8 60 20 60 20 69 b 20 78 8 78 8 69 m 48 91 b 48 82 60 82 60 91 b 60 100 48 100 48 91 m 83 62 b 83 53 95 53 95 62 b 95 71 83 71 83 62 m 64 17 b 64 8 76 8 76 17 b 76 26 64 26 64 17 m 44 52 b 44 43 56 43 56 52 b 56 61 44 61 44 52 ",
			flower4t 	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 b 0 55 12 20 50 53 b 31 7 70 7 50 53 b 87 20 100 55 50 53 b 93 79 62 100 50 53 b 38 100 7 79 50 53 m 5 39 b 5 32 15 32 15 39 b 15 46 5 46 5 39 m 84 39 b 84 32 94 32 94 39 b 94 46 84 46 84 39 m 45 11 b 45 4 55 4 55 11 b 55 18 45 18 45 11 m 21 87 b 21 80 31 80 31 87 b 31 94 21 94 21 87 m 70 87 b 70 80 80 80 80 87 b 80 94 70 94 70 87 ",
			flower5t 	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 b 0 55 12 20 50 53 b 31 7 70 7 50 53 b 87 20 100 55 50 53 b 93 79 62 100 50 53 b 38 100 7 79 50 53 ",
			flower6t 	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 b 38 69 46 91 23 91 b 16 69 38 69 50 53 b 31 46 14 61 6 38 b 24 24 31 46 50 53 b 50 33 31 20 50 6 b 69 20 50 33 50 53 b 69 46 75 25 94 38 b 87 61 69 46 50 53 b 62 69 85 68 78 91 b 54 91 62 69 50 53 m 44 19 b 44 28 56 28 56 19 b 56 10 44 10 44 19 m 24 80 b 24 89 36 89 36 80 b 36 71 24 71 24 80 m 64 80 b 64 89 76 89 76 80 b 76 71 64 71 64 80 m 12 42 b 12 51 24 51 24 42 b 24 33 12 33 12 42 m 76 42 b 76 51 88 51 88 42 b 88 33 76 33 76 42 m 40 53 b 40 67 60 67 60 53 b 60 40 40 40 40 53 ",
			flower7t 	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 b 38 69 46 91 23 91 b 16 69 38 69 50 53 b 31 46 14 61 6 38 b 24 24 31 46 50 53 b 50 33 31 20 50 6 b 69 20 50 33 50 53 b 69 46 75 25 94 38 b 87 61 69 46 50 53 b 62 69 85 68 78 91 b 54 91 62 69 50 53 m 44 19 b 44 28 56 28 56 19 b 56 10 44 10 44 19 m 24 80 b 24 89 36 89 36 80 b 36 71 24 71 24 80 m 64 80 b 64 89 76 89 76 80 b 76 71 64 71 64 80 m 12 42 b 12 51 24 51 24 42 b 24 33 12 33 12 42 m 76 42 b 76 51 88 51 88 42 b 88 33 76 33 76 42 ",
			flower8t 	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 b 38 69 46 91 23 91 b 16 69 38 69 50 53 b 31 46 14 61 6 38 b 24 24 31 46 50 53 b 50 33 31 20 50 6 b 69 20 50 33 50 53 b 69 46 75 25 94 38 b 87 61 69 46 50 53 b 62 69 85 68 78 91 b 54 91 62 69 50 53 ",
			flower9t 	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 ",
			flower10t	= "m 50 53 b 31 59 10 66 20 95 b 50 95 50 73 50 53 b 50 73 50 95 81 95 b 90 66 69 59 50 53 b 69 59 90 66 100 36 b 75 18 62 36 50 53 b 62 36 75 18 50 0 b 25 18 38 36 50 53 b 38 36 25 18 0 36 b 10 66 31 59 50 53 b 0 55 12 20 50 53 b 31 7 70 7 50 53 b 87 20 100 55 50 53 b 93 79 62 100 50 53 b 38 100 7 79 50 53 m 11 64 b 11 71 21 71 21 64 b 21 57 11 57 11 64 m 80 64 b 80 71 90 71 90 64 b 90 57 80 57 80 64 m 45 89 b 45 96 55 96 55 89 b 55 82 45 82 45 89 m 24 23 b 24 30 34 30 34 23 b 34 16 24 16 24 23 m 66 23 b 66 30 76 30 76 23 b 76 16 66 16 66 23 ",
			flower11t	= "m 100 88 b 79 99 57 89 57 62 b 80 75 100 37 100 88 m 49 66 b 49 92 92 92 49 116 b 28 104 25 80 49 66 m 41 62 b 19 75 38 110 0 88 b 0 65 18 48 41 62 m 41 54 b 20 40 0 77 0 28 b 19 17 41 26 41 54 m 49 49 b 49 23 8 23 49 0 b 68 11 71 36 49 49 m 57 54 b 80 40 58 5 100 28 b 100 54 81 67 57 54 ",
			flower12t	= "m 0 87 b 21 99 43 89 43 62 b 20 75 0 38 0 87 m 50 66 b 50 92 8 92 50 116 b 71 103 74 80 50 66 m 57 62 b 80 75 61 109 100 87 b 100 65 81 49 57 62 m 57 54 b 79 41 100 77 100 29 b 80 17 57 27 57 54 m 50 50 b 50 24 92 24 50 0 b 30 11 27 37 50 50 m 43 54 b 20 41 42 4 0 29 b 0 54 19 67 43 54 m 44 58 b 44 66 56 66 56 58 b 56 50 44 50 44 58 m 24 82 b 24 90 12 90 12 82 b 12 74 24 74 24 82 m 60 97 b 60 105 48 105 48 97 b 48 89 60 89 60 97 m 93 76 b 93 84 81 84 81 76 b 81 68 93 68 93 76 m 91 34 b 91 42 79 42 79 34 b 79 26 91 26 91 34 m 54 17 b 54 25 42 25 42 17 b 42 9 54 9 54 17 m 20 39 b 20 47 8 47 8 39 b 8 31 20 31 20 39 ",
			flower13t	= "m 0 87 b 21 99 43 89 43 62 b 20 75 0 38 0 87 m 50 66 b 50 92 8 92 50 116 b 71 103 74 80 50 66 m 57 62 b 80 75 61 109 100 87 b 100 65 81 49 57 62 m 57 54 b 79 41 100 77 100 29 b 80 17 57 27 57 54 m 50 50 b 50 24 92 24 50 0 b 30 11 27 37 50 50 m 43 54 b 20 41 42 4 0 29 b 0 54 19 67 43 54 m 44 58 b 44 66 56 66 56 58 b 56 50 44 50 44 58 ",
			flower14t	= "m 0 87 b 21 99 43 89 43 62 b 20 75 0 38 0 87 m 50 66 b 50 92 8 92 50 116 b 71 103 74 80 50 66 m 57 62 b 80 75 61 109 100 87 b 100 65 81 49 57 62 m 57 54 b 79 41 100 77 100 29 b 80 17 57 27 57 54 m 50 50 b 50 24 92 24 50 0 b 30 11 27 37 50 50 m 43 54 b 20 41 42 4 0 29 b 0 54 19 67 43 54 ",
			flower15t	= "m 30 71 l 33 62 b 16 59 0 59 0 88 b 24 102 33 88 38 74 l 30 71 m 50 84 l 44 77 b 33 88 24 102 50 117 b 76 102 68 88 56 77 l 50 84 m 70 71 l 62 74 b 68 88 76 102 100 88 b 100 59 84 59 67 62 l 70 71 m 70 47 l 67 56 b 84 59 100 59 100 30 b 76 16 67 30 62 45 l 70 47 m 50 35 l 56 42 b 67 30 76 16 50 1 b 24 16 33 30 44 42 l 50 35 m 30 47 l 38 45 b 33 30 24 16 0 30 b 0 59 16 59 33 56 l 30 47 m 0 35 b 0 25 14 25 14 35 b 14 45 0 45 0 35 m 0 83 b 0 73 14 73 14 83 b 14 93 0 93 0 83 m 43 108 b 43 98 57 98 57 108 b 57 118 43 118 43 108 m 43 10 b 43 0 57 0 57 10 b 57 20 43 20 43 10 m 86 83 b 86 73 100 73 100 83 b 100 93 86 93 86 83 m 86 35 b 86 25 100 25 100 35 b 100 45 86 45 86 35 ",
			flower16t	= "m 30 70 l 33 61 b 16 58 0 58 0 87 b 24 102 33 87 38 73 l 30 70 m 50 83 l 44 76 b 33 87 24 102 50 116 b 76 102 68 87 56 76 l 50 83 m 70 70 l 62 73 b 68 87 76 102 100 87 b 100 58 84 58 67 61 l 70 70 m 70 46 l 67 55 b 84 58 100 58 100 29 b 76 14 67 29 62 44 l 70 46 m 50 34 l 56 41 b 67 29 76 14 50 0 b 24 14 33 29 44 41 l 50 34 m 30 46 l 38 44 b 33 29 24 14 0 29 b 0 58 16 58 33 55 l 30 46 m 26 29 b 26 19 40 19 40 29 b 40 39 26 39 26 29 m 60 29 b 60 19 74 19 74 29 b 74 39 60 39 60 29 m 9 58 b 9 48 23 48 23 58 b 23 68 9 68 9 58 m 77 58 b 77 48 91 48 91 58 b 91 68 77 68 77 58 m 26 87 b 26 77 40 77 40 87 b 40 97 26 97 26 87 m 60 87 b 60 77 74 77 74 87 b 74 97 60 97 60 87 ",
			flower17t	= "m 30 70 l 33 61 b 16 58 0 58 0 87 b 24 102 33 87 38 73 l 30 70 m 50 83 l 44 76 b 33 87 24 102 50 116 b 76 102 68 87 56 76 l 50 83 m 70 70 l 62 73 b 68 87 76 102 100 87 b 100 58 84 58 67 61 l 70 70 m 70 46 l 67 55 b 84 58 100 58 100 29 b 76 14 67 29 62 44 l 70 46 m 50 34 l 56 41 b 67 29 76 14 50 0 b 24 14 33 29 44 41 l 50 34 m 30 46 l 38 44 b 33 29 24 14 0 29 b 0 58 16 58 33 55 l 30 46 ",
			flower18t	= "m 30 70 l 33 61 b 16 58 0 58 0 87 b 24 102 33 87 38 73 l 30 70 m 50 82 l 43 75 b 33 87 24 102 50 116 b 76 102 68 87 57 75 l 50 82 m 70 70 l 62 73 b 68 87 76 102 100 87 b 100 58 84 58 67 61 l 70 70 m 70 46 l 67 55 b 84 58 100 58 100 29 b 76 14 67 29 62 44 l 70 46 m 50 34 l 57 42 b 67 29 76 14 50 0 b 24 14 33 29 43 42 l 50 34 m 30 46 l 38 44 b 33 29 24 14 0 29 b 0 58 16 58 33 55 l 30 46 m 58 34 l 42 34 l 50 21 l 58 34 m 82 40 l 74 53 l 66 40 l 82 40 m 34 40 l 26 53 l 18 40 l 34 40 m 58 82 l 50 95 l 42 82 l 58 82 m 34 76 l 18 76 l 26 63 l 34 76 m 82 76 l 66 76 l 74 63 l 82 76 m 5 35 b 5 28 15 28 15 35 b 15 42 5 42 5 35 m 5 81 b 5 74 15 74 15 81 b 15 88 5 88 5 81 m 85 81 b 85 74 95 74 95 81 b 95 88 85 88 85 81 m 85 35 b 85 28 95 28 95 35 b 95 42 85 42 85 35 m 45 104 b 45 97 55 97 55 104 b 55 111 45 111 45 104 m 45 12 b 45 5 55 5 55 12 b 55 19 45 19 45 12 ",
			flower19t	= "m 30 70 l 33 61 b 16 58 0 58 0 87 b 24 102 33 87 38 73 l 30 70 m 50 82 l 43 75 b 33 87 24 102 50 116 b 76 102 68 87 57 75 l 50 82 m 70 70 l 62 73 b 68 87 76 102 100 87 b 100 58 84 58 67 61 l 70 70 m 70 46 l 67 55 b 84 58 100 58 100 29 b 76 14 67 29 62 44 l 70 46 m 50 34 l 57 42 b 67 29 76 14 50 0 b 24 14 33 29 43 42 l 50 34 m 30 46 l 38 44 b 33 29 24 14 0 29 b 0 58 16 58 33 55 l 30 46 m 58 34 l 42 34 l 50 21 l 58 34 m 82 40 l 74 53 l 66 40 l 82 40 m 34 40 l 26 53 l 18 40 l 34 40 m 58 82 l 50 95 l 42 82 l 58 82 m 34 76 l 18 76 l 26 63 l 34 76 m 82 76 l 66 76 l 74 63 l 82 76 ",
			flower20t	= "m 30 70 l 33 58 b 16 58 0 58 0 87 b 24 102 33 87 41 72 l 30 70 m 50 81 l 41 72 b 33 87 24 102 50 116 b 76 102 68 87 59 72 l 50 81 m 70 70 l 59 72 b 68 87 76 102 100 87 b 100 58 84 58 67 58 l 70 70 m 70 46 l 67 58 b 84 58 100 58 100 29 b 76 14 67 29 59 43 l 70 46 m 50 34 l 59 43 b 67 29 76 14 50 0 b 24 14 33 29 41 43 l 50 34 m 30 46 l 41 43 b 33 29 24 14 0 29 b 0 58 16 58 33 58 l 30 46 m 19 43 b 19 35 31 35 31 43 b 31 51 19 51 19 43 m 19 73 b 19 65 31 65 31 73 b 31 81 19 81 19 73 m 69 43 b 69 35 81 35 81 43 b 81 51 69 51 69 43 m 69 73 b 69 65 81 65 81 73 b 81 81 69 81 69 73 m 44 87 b 44 79 56 79 56 87 b 56 95 44 95 44 87 m 44 28 b 44 20 56 20 56 28 b 56 36 44 36 44 28 m 44 58 b 44 50 56 50 56 58 b 56 66 44 66 44 58 m 0 58 b 0 50 12 50 12 58 b 12 66 0 66 0 58 m 88 58 b 88 50 100 50 100 58 b 100 66 88 66 88 58 m 22 96 b 22 88 34 88 34 96 b 34 104 22 104 22 96 m 66 20 b 66 12 78 12 78 20 b 78 28 66 28 66 20 m 22 20 b 22 12 34 12 34 20 b 34 28 22 28 22 20 m 67 96 b 67 88 79 88 79 96 b 79 104 67 104 67 96 ",
			flower21t	= "m 30 70 l 33 58 b 16 58 0 58 0 87 b 24 102 33 87 41 72 l 30 70 m 50 81 l 41 72 b 33 87 24 102 50 116 b 76 102 68 87 59 72 l 50 81 m 70 70 l 59 72 b 68 87 76 102 100 87 b 100 58 84 58 67 58 l 70 70 m 70 46 l 67 58 b 84 58 100 58 100 29 b 76 14 67 29 59 43 l 70 46 m 50 34 l 59 43 b 67 29 76 14 50 0 b 24 14 33 29 41 43 l 50 34 m 30 46 l 41 43 b 33 29 24 14 0 29 b 0 58 16 58 33 58 l 30 46 m 0 58 b 0 50 12 50 12 58 b 12 66 0 66 0 58 m 88 58 b 88 50 100 50 100 58 b 100 66 88 66 88 58 m 22 96 b 22 88 34 88 34 96 b 34 104 22 104 22 96 m 66 20 b 66 12 78 12 78 20 b 78 28 66 28 66 20 m 22 20 b 22 12 34 12 34 20 b 34 28 22 28 22 20 m 67 96 b 67 88 79 88 79 96 b 79 104 67 104 67 96 ",
			flower22t	= "m 30 70 l 33 58 b 16 58 0 58 0 87 b 24 102 33 87 41 72 l 30 70 m 50 81 l 41 72 b 33 87 24 102 50 116 b 76 102 68 87 59 72 l 50 81 m 70 70 l 59 72 b 68 87 76 102 100 87 b 100 58 84 58 67 58 l 70 70 m 70 46 l 67 58 b 84 58 100 58 100 29 b 76 14 67 29 59 43 l 70 46 m 50 34 l 59 43 b 67 29 76 14 50 0 b 24 14 33 29 41 43 l 50 34 m 30 46 l 41 43 b 33 29 24 14 0 29 b 0 58 16 58 33 58 l 30 46 m 19 43 b 19 35 31 35 31 43 b 31 51 19 51 19 43 m 19 73 b 19 65 31 65 31 73 b 31 81 19 81 19 73 m 69 43 b 69 35 81 35 81 43 b 81 51 69 51 69 43 m 69 73 b 69 65 81 65 81 73 b 81 81 69 81 69 73 m 44 87 b 44 79 56 79 56 87 b 56 95 44 95 44 87 m 44 28 b 44 20 56 20 56 28 b 56 36 44 36 44 28 m 44 58 b 44 50 56 50 56 58 b 56 66 44 66 44 58 ",
			flower23t	= "m 30 70 l 33 58 b 16 58 0 58 0 87 b 24 102 33 87 41 72 l 30 70 m 50 81 l 41 72 b 33 87 24 102 50 116 b 76 102 68 87 59 72 l 50 81 m 70 70 l 59 72 b 68 87 76 102 100 87 b 100 58 84 58 67 58 l 70 70 m 70 46 l 67 58 b 84 58 100 58 100 29 b 76 14 67 29 59 43 l 70 46 m 50 34 l 59 43 b 67 29 76 14 50 0 b 24 14 33 29 41 43 l 50 34 m 30 46 l 41 43 b 33 29 24 14 0 29 b 0 58 16 58 33 58 l 30 46 m 56 48 l 78 42 l 61 58 l 78 75 l 56 67 l 50 88 l 44 67 l 23 75 l 39 58 l 23 42 l 44 48 l 50 28 l 56 48 ",
			flower24t	= "m 30 70 l 33 58 b 16 58 0 58 0 87 b 24 102 33 87 41 72 l 30 70 m 50 81 l 41 72 b 33 87 24 102 50 116 b 76 102 68 87 59 72 l 50 81 m 70 70 l 59 72 b 68 87 76 102 100 87 b 100 58 84 58 67 58 l 70 70 m 70 46 l 67 58 b 84 58 100 58 100 29 b 76 14 67 29 59 43 l 70 46 m 50 34 l 59 43 b 67 29 76 14 50 0 b 24 14 33 29 41 43 l 50 34 m 30 46 l 41 43 b 33 29 24 14 0 29 b 0 58 16 58 33 58 l 30 46 m 56 48 l 94 33 l 61 58 l 94 84 l 56 67 l 50 109 l 44 67 l 6 84 l 39 58 l 6 33 l 44 48 l 50 8 l 56 48 ",
			flower25t	= "m 30 70 l 33 58 b 16 58 0 58 0 87 b 24 102 33 87 41 72 l 30 70 m 50 81 l 41 72 b 33 87 24 102 50 116 b 76 102 68 87 59 72 l 50 81 m 70 70 l 59 72 b 68 87 76 102 100 87 b 100 58 84 58 67 58 l 70 70 m 70 46 l 67 58 b 84 58 100 58 100 29 b 76 14 67 29 59 43 l 70 46 m 50 34 l 59 43 b 67 29 76 14 50 0 b 24 14 33 29 41 43 l 50 34 m 30 46 l 41 43 b 33 29 24 14 0 29 b 0 58 16 58 33 58 l 30 46 ",
			flower26t	= "m 50 55 b 33 98 44 110 50 110 b 54 110 64 98 50 55 b 78 88 95 86 97 83 b 100 79 94 63 50 55 b 6 63 0 79 2 83 b 4 86 21 88 50 55 b 21 23 4 25 2 28 b 0 31 6 46 50 55 b 94 46 100 31 97 28 b 95 25 78 23 50 55 b 64 14 53 0 50 0 b 45 0 33 14 50 55 m 100 55 b 100 42 83 42 83 55 b 83 68 100 68 100 55 m 17 55 b 17 42 0 42 0 55 b 0 68 17 68 17 55 m 78 91 b 78 78 61 78 61 91 b 61 103 78 103 78 91 m 37 91 b 37 78 20 78 20 91 b 20 103 37 103 37 91 m 78 19 b 78 7 61 7 61 19 b 61 32 78 32 78 19 m 37 19 b 37 7 20 7 20 19 b 20 32 37 32 37 19 m 62 55 b 62 38 38 38 38 55 b 38 72 62 72 62 55 ",
			flower27t	= "m 50 55 b 35 98 45 110 50 110 b 55 110 65 98 50 55 b 79 88 96 87 98 83 b 100 80 95 63 50 55 b 7 63 0 80 2 83 b 4 87 21 88 50 55 b 21 23 4 25 2 28 b 0 31 7 47 50 55 b 95 47 100 31 98 28 b 96 25 79 23 50 55 b 65 14 55 0 50 0 b 45 0 35 14 50 55 m 62 55 b 62 38 38 38 38 55 b 38 72 62 72 62 55 ",
			flower28t	= "m 50 55 b 35 98 45 110 50 110 b 55 110 65 98 50 55 b 79 88 96 87 98 83 b 100 80 95 63 50 55 b 7 63 0 80 2 83 b 4 87 21 88 50 55 b 21 23 4 25 2 28 b 0 31 7 47 50 55 b 95 47 100 31 98 28 b 96 25 79 23 50 55 b 65 14 55 0 50 0 b 45 0 35 14 50 55 ",
			flower29t	= "m 50 55 b 33 98 44 110 50 110 b 54 110 64 98 50 55 b 78 88 95 86 97 83 b 100 79 94 63 50 55 b 6 63 0 79 2 83 b 4 86 21 88 50 55 b 21 23 4 25 2 28 b 0 31 6 46 50 55 b 94 46 100 31 97 28 b 95 25 78 23 50 55 b 64 14 53 0 50 0 b 45 0 33 14 50 55 m 100 55 b 100 42 83 42 83 55 b 83 68 100 68 100 55 m 17 55 b 17 42 0 42 0 55 b 0 68 17 68 17 55 m 78 91 b 78 78 61 78 61 91 b 61 103 78 103 78 91 m 37 91 b 37 78 20 78 20 91 b 20 103 37 103 37 91 m 78 19 b 78 7 61 7 61 19 b 61 32 78 32 78 19 m 37 19 b 37 7 20 7 20 19 b 20 32 37 32 37 19 ",
			cristal17	= "m 56 50 l 64 100 l 38 78 l 56 50 m 56 50 l 24 100 l 0 65 l 56 50 m 56 50 l 0 65 l 28 39 l 56 50 m 56 50 l 31 0 l 0 28 l 56 50 m 56 50 l 78 0 l 42 22 l 56 50 m 56 50 l 100 40 l 68 23 l 56 50 m 56 50 l 100 40 l 61 80 l 56 50 m 100 40 l 61 80 l 56 50 m 100 40 l 77 64 l 100 100 l 100 40 m 61 80 l 100 100 l 77 64 l 61 80 m 61 80 l 64 100 l 100 100 l 61 80 m 38 78 l 24 100 l 64 100 l 38 78 m 0 65 l 0 100 l 24 100 l 0 65 m 0 28 l 0 65 l 28 39 l 0 28 m 31 0 l 0 0 l 0 28 l 31 0 m 78 0 l 31 0 l 42 22 l 78 0 m 100 0 l 68 23 l 100 40 l 100 0 m 100 0 l 78 0 l 68 23 l 100 0 ",
			geometric10 = "m 0 50 l 0 100 l 100 100 l 100 50 l 90 50 l 90 90 l 10 90 l 10 50 l 0 50 m 0 50 l 10 50 l 10 10 l 90 10 l 90 50 l 100 50 l 100 0 l 0 0 l 0 50 m 50 90 l 50 80 l 20 80 l 20 20 l 50 20 l 50 10 l 10 10 l 10 90 l 50 90 m 50 90 l 90 90 l 90 10 l 50 10 l 50 20 l 80 20 l 80 80 l 50 80 l 50 90 m 20 50 l 20 80 l 80 80 l 80 50 l 70 50 l 70 70 l 30 70 l 30 50 l 20 50 m 20 50 l 30 50 l 30 30 l 70 30 l 70 50 l 80 50 l 80 20 l 20 20 l 20 50 m 50 70 l 50 60 l 40 60 l 40 40 l 50 40 l 50 30 l 30 30 l 30 70 l 50 70 m 50 70 l 70 70 l 70 30 l 50 30 l 50 40 l 60 40 l 60 60 l 50 60 l 50 70 m 40 50 l 40 60 l 60 60 l 60 50 l 40 50 m 40 50 l 60 50 l 60 40 l 40 40 l 40 50 ",
			diagonal13r = "m 15 0 l 0 0 l 0 15 l 15 0 m 15 0 l 0 15 l 0 30 l 30 0 l 15 0 m 30 0 l 0 30 l 0 45 l 45 0 l 30 0 m 45 0 l 0 45 l 0 60 l 60 0 l 45 0 m 60 0 l 0 60 l 0 75 l 75 0 l 60 0 m 75 0 l 0 75 l 0 90 l 90 0 l 75 0 m 90 0 l 0 90 l 0 100 l 5 100 l 100 5 l 100 0 l 90 0 m 100 5 l 5 100 l 20 100 l 100 20 l 100 5 m 100 20 l 20 100 l 35 100 l 100 35 l 100 20 m 100 35 l 35 100 l 50 100 l 100 50 l 100 35 m 100 50 l 50 100 l 65 100 l 100 65 l 100 50 m 100 65 l 65 100 l 80 100 l 100 80 l 100 65 m 100 80 l 80 100 l 100 100 l 100 80 ",
			diagonal13l	= "m 0 85 l 0 100 l 15 100 l 0 85 m 0 85 l 15 100 l 30 100 l 0 70 l 0 85 m 0 70 l 30 100 l 45 100 l 0 55 l 0 70 m 0 55 l 45 100 l 60 100 l 0 40 l 0 55 m 0 40 l 60 100 l 75 100 l 0 25 l 0 40 m 0 25 l 75 100 l 90 100 l 0 10 l 0 25 m 0 10 l 90 100 l 100 100 l 100 95 l 5 0 l 0 0 l 0 10 m 5 0 l 100 95 l 100 80 l 20 0 l 5 0 m 20 0 l 100 80 l 100 65 l 35 0 l 20 0 m 35 0 l 100 65 l 100 50 l 50 0 l 35 0 m 50 0 l 100 50 l 100 35 l 65 0 l 50 0 m 65 0 l 100 35 l 100 20 l 80 0 l 65 0 m 80 0 l 100 20 l 100 0 l 80 0 ",
			----------------------------------------------------
			
			ASSDraw3 = function( Shape, Round )
				--le da formato a las shapes para usar en Auto-4
				local Round = Round or 2 --cifras decimales a redondear los números en la shape
				local Shape = Shape or "m 0 0 l 0 100 l 100 100 l 100 0 l 0 0 "
				if type( Shape ) == "table" then
					for i = 1, #Shape do
						Shape[ i ] = ke4.shape.ASSDraw3( Shape[ i ], Round )
					end --recursividad: september 08th 2019
				else
					Shape = Shape:gsub( "  ", " " ) -- elimina los espacios múltiples
					Shape = Shape:gsub( "%S+",
						function( num )
							return format( "%s", ke4.math.round( tonumber( num ) or num, Round ) )
						end
					)
					Shape = Shape:gsub( " c", "" ):gsub( "%b{}", "" )
					local segments, coor2 = { }, { }
					if Shape:match( "%i?clip%b()" ) then
						if Shape:match( "%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d]*" ) then
							local cx1, cy1, cx2, cy2 = Shape:match( "(%-?%d+[%.%d ]*)%,[ ]*(%-?%d+[%.%d ]*)%,[ ]*(%-?%d+[%.%d ]*)%,[ ]*(%-?%d+[%.%d]*)" )
							Shape = format( "m %s %s l %s %s l %s %s l %s %s ", cx1, cy1, cx1, cy2, cx2, cy2, cx2, cy1 )
						elseif Shape:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dmlb ]*" ) then
							Shape = Shape:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dmlb ]*" )
						end
					end
					for c in Shape:gmatch( "[mlb]^* %-?%d+[%.%d]* [%-%.%d ]*" ) do
						table.insert( segments, c )
					end
					for k = 1, #segments do
						coor2[ k ] = { }
						for c6 in segments[ k ]:gmatch( "%S+" ) do
							--captura: signos, números, letras y puntos
							table.insert( coor2[ k ], format( "%s ", c6 ) )
						end
					end
					for k = 1, #coor2 do
						if coor2[ k ][ 1 ] == "b "
							and #coor2[ k ] > 7 then
							coor3 = { }
							table.remove( coor2[ k ], 1 )
							for i = 1, #coor2[ k ] / 6 do
								coor3[ i ] = { }
								for h = 1, 6 do
									table.insert( coor3[ i ], coor2[ k ][ 6 * (i - 1) + h ] )
								end
								coor3[ i ] = format( "b %s", ke4.table.op( coor3[ i ], "concat" ) )
							end
							coor2[ k ] = ke4.table.op( coor3, "concat" )
						elseif coor2[ k ][ 1 ] == "l "
							and #coor2[ k ] > 3 then
							coor4 = { }
							table.remove( coor2[ k ], 1 )
							for i = 1, #coor2[ k ] / 2 do
								coor4[ i ] = { }
								for h = 1, 2 do
									table.insert( coor4[ i ], coor2[ k ][ 2 * (i - 1) + h ] )
								end
								coor4[ i ] = format( "l %s", ke4.table.op( coor4[ i ], "concat" ) )
							end
							coor2[ k ] = ke4.table.op( coor4, "concat" )
						else
							coor2[ k ] = ke4.table.op( coor2[ k ], "concat" )
						end
					end
					Shape = ke4.table.op( coor2, "concat" )
				end
				return Shape
			end,
			
			round = function( Shape, Round )
				--redondea los valores de la Shape a las cifras decimales indicadas o al entero más cercano
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Round = Round or 0
				if type( Shape ) == "table" then
					for i = 1, #Shape do
						Shape[ i ] = ke4.shape.round( Shape[ i ], Round )
					end
				else
					Shape = Shape:gsub( "%-?%d+[%.%d]*",
						function( num )
							return ke4.math.round( tonumber( num ), Round )
						end
					)
				end
				return Shape
			end,
			
			info = function( Shape )
				--genera variables con la información notable de la Shape
				local Shape = ke4.shape.ASSDraw3( Shape )
				local shape_coor = { x = { }, y = { } }
				for coor_x, coor_y in Shape:gmatch( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)" ) do
					table.insert( shape_coor.x, tonumber( coor_x ) )
					table.insert( shape_coor.y, tonumber( coor_y ) )
				end
				-- Shape Data -------------------
				shape_x2 = ke4.table.duplicate( shape_coor.x )	--> la tabla de las coordenadas en "x"
				shape_y2 = ke4.table.duplicate( shape_coor.y )	--> la tabla de las coordenadas en "y"
				minx = ke4.table.op( shape_coor.x, "min" )		--> la menor de las coordenadas en "x"
				maxx = ke4.table.op( shape_coor.x, "max" )		--> la mayor de las coordenadas en "x"
				miny = ke4.table.op( shape_coor.y, "min" )		--> la menor de las coordenadas en "y"
				maxy = ke4.table.op( shape_coor.y, "max" )		--> la mayor de las coordenadas en "y"
				w_shape = maxx - minx							--> (width) ancho total en pixeles de la shape
				h_shape = maxy - miny							--> (height) alto total en pixeles de la shape
				c_shape = minx + w_shape / 2					--> (center) coordenada en "x" del centro de la shape
				m_shape = miny + h_shape / 2					--> (middle) coordenada en "y" del centro de la shape
				n_shape = #shape_coor.x							--> (points) cantidad de puntos de la shape
				n_points = #shape_coor.x						--> (points) cantidad de puntos de la shape
				---------------------------------
			end,
			
			redraw = function( Shape, tract, Section )
				--redibuja la Shape de forma que cada parte que la conforme
				--"line" o "bezier" sea seccionada en partes iguales (tract)
				local Shape = ke4.shape.ASSDraw3( Shape )
				local tract = tract or 2
				local shape_parts, shape_sgm, shape_redraw = { }, { }, { }
				local shape_new = recall.shprd
				local length, angle_, n, N
				local Section = Section or "all"
				--if j == 1 then
				--if recall.shprd == nil then
					shape_new = ""
					for c in Shape:gmatch( "[mlb]^* %-?%d+[%.%d]* [%-%.%d ]*" ) do
						table.insert( shape_parts, c )
					end
					for i = 1, #shape_parts do
						shape_sgm[ i ] = { }
						for c in shape_parts[ i ]:gmatch( "%S+" ) do
							table.insert( shape_sgm[ i ], tonumber( c ) or c )
						end
					end
					for i = 1, #shape_sgm do
						shape_redraw[ i ] = { }
						if shape_sgm[ i ][ 1 ] == "m" then
							shape_redraw[ i ] = shape_sgm[ i ]
						elseif shape_sgm[ i ][ 1 ] == "l" then
							if Section ~= "bezier" then
								n = #shape_sgm[ i - 1 ]
								length = ke4.math.distance( shape_sgm[ i - 1 ][ n - 1 ], shape_sgm[ i - 1 ][ n ], shape_sgm[ i ][ 2 ], shape_sgm[ i ][ 3 ] )
								angle_ = ke4.math.angle( shape_sgm[ i - 1 ][ n - 1 ], shape_sgm[ i - 1 ][ n ], shape_sgm[ i ][ 2 ], shape_sgm[ i ][ 3 ] )
								N = ceil( length / tract )
								shape_redraw[ i ][ -1 ] = shape_sgm[ i - 1 ][ n - 1 ]
								shape_redraw[ i ][ 0 ]  = shape_sgm[ i - 1 ][ n ]
								for k = 1, N do
									local Px = shape_sgm[ i - 1 ][ n - 1 ] + ke4.math.polar( angle_, length * k / N, "x" )
									local Py = shape_sgm[ i - 1 ][ n - 0 ] + ke4.math.polar( angle_, length * k / N, "y" )
									shape_redraw[ i ][ k ] = format( "%s %s", Px, Py )
								end
								table.insert( shape_redraw[ i ], 1, "l" )
							else
								shape_redraw[ i ] = shape_sgm[ i ]
							end
						elseif shape_sgm[ i ][ 1 ] == "b" then
							if Section ~= "line" then
								n = #shape_sgm[ i - 1 ]
								local Bx = { shape_sgm[ i - 1 ][ n - 1 ], shape_sgm[ i ][ 2 ], shape_sgm[ i ][ 4 ], shape_sgm[ i ][ 6 ] }
								local By = { shape_sgm[ i - 1 ][ n - 0 ], shape_sgm[ i ][ 3 ], shape_sgm[ i ][ 5 ], shape_sgm[ i ][ 7 ] }
								length = ke4.math.length_bezier( shape_sgm[ i - 1 ][ n - 1 ],  shape_sgm[ i - 1 ][ n ], shape_sgm[ i ][ 2 ], 
									shape_sgm[ i ][ 3 ], shape_sgm[ i ][ 4 ], shape_sgm[ i ][ 5 ], shape_sgm[ i ][ 6 ], shape_sgm[ i ][ 7 ] )
								N = ceil( length / tract )
								for k = 1, N do
									local Px, Py = ke4.math.confi_bezier( 4, Bx, By, k / N )
									shape_redraw[ i ][ k ] = format( "%s %s", Px, Py )
								end
								table.insert( shape_redraw[ i ], 1, "l" )
							else
								shape_redraw[ i ] = shape_sgm[ i ]
							end
						end
					end
					for i = 1, #shape_redraw do
						for k = 1, #shape_redraw[ i ] do
							shape_new = shape_new .. shape_redraw[ i ][ k ] .. " "
						end
					end
					shape_new = remember( "shprd", shape_new )
				--end--
				return ke4.shape.ASSDraw3( shape_new )
			end, --{\p1}!_G.ke4.shape.redraw( _G.ke4.shape.circle, 5 )!

			modify = function( Shape, Modify )
				--le aplica un "filtro" (función) a los valores numéricos de la Shape
				local Shape = ke4.shape.ASSDraw3( Shape )
				Pk = 0
				ke4.shape.info( Shape )
				local Modify = Modify or function( x, y )
					return x, y
				end
				Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)",
					function( x, y )
						x, y = tonumber( x ), tonumber( y )		--> Coordenadas "x" y "y" de cada punto
						Cx = c_shape							--> coordenada "x" del centro de la shape
						Cy = m_shape							--> coordenada "y" del centro de la shape
						Do = ke4.math.distance( x, y )			--> distancia del punto al origen
						Dc = ke4.math.distance( Cx, Cy, x, y )	--> distancia del punto al centro de la shape
						Ao = ke4.math.angle( x, y )				--> ángulo del origen al punto
						Ac = ke4.math.angle( Cx, Cy, x, y )		--> ángulo del centro al punto
						Pn = n_points							--> cantidad total de puntos en la shape
						Pk = Pk + 1								--> contador de los puntos de la shape
						Mx = (y - miny ) / h_shape				--> módulo de varianza respecto a "x", Mx = [0, 1]
						My = (x - minx ) / w_shape				--> módulo de varianza respecto a "y", My = [0, 1]
						Mp = (Pk - 1) / (Pn - 1)				--> módulo de varianza respecto a los puntos, Mp = [0, 1]
						local mod_x, mod_y = Modify( x, y )
						if mod_y then
							return format( "%s %s", Modify( x, y ) )
						end
						return Modify( x, y )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.modify( _G.ke4.shape.redraw( "m 0 0 l 0 50 l 50 50 l 50 0 l 0 0 ", 1 ), function( x, y ) x = x + _G.ke4.math.Rcs( 4 ) y = y + _G.ke4.math.Rcs( 4 ) return x, y end )!
			
			filter7 = function( Shape, Filter )
				--le aplica un "filtro" (función) a los valores numéricos de la Shape
				local Shape = ke4.shape.ASSDraw3( Shape )
				Pk = 0
				ke4.shape.info( Shape )
				Shape = ke4.shape.filtery( Shape,
					function( x, y )
						x, y = tonumber( x ), tonumber( y )		-- coordenadas "x" y "y" de cada punto
						Cx = c_shape							-- coordenada "x" del centro de la shape
						Cy = m_shape							-- coordenada "y" del centro de la shape
						Do = ke4.math.distance( x, y )			-- distancia del punto al origen
						Dc = ke4.math.distance( Cx, Cy, x, y )	-- distancia del punto al centro de la shape
						Ao = ke4.math.angle( x, y )				-- ángulo del origen al punto
						Ac = ke4.math.angle( Cx, Cy, x, y )		-- ángulo del centro al punto
						Pn = n_points							-- cantidad total de puntos en la shape
						Pk = Pk + 1								-- contador de los puntos de la shape
						Mx = (y - miny ) / h_shape				-- módulo de varianza respecto a "x", Mx = [0, 1]
						My = (x - minx ) / w_shape				-- módulo de varianza respecto a "y", My = [0, 1]
						Mp = (Pk - 1) / (Pn - 1)				-- módulo de varianza respecto a los puntos, Mp = [0, 1]
						local toval
						if pcall( loadstring(
							format( [[
								return function( x, y )
									%s
									return x, y
								end
								]], Filter )
							) ) then
							local tofunct = loadstring(
							format( [[
								return function( x, y )
									%s
									return x, y
								end
								]], Filter )
							)( )
							if pcall( tofunct ) then
								x, y = tofunct( x, y )
							end
						end
						return x, y
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end, --!_G.ke4.shape.filter7( _G.ke4.shape.rectangle, "x = 2 * x" )!

			filter = function( Shape, Filter )
				--le aplica un "filtro" (función) a los valores numéricos de la Shape
				local Shape = ke4.shape.ASSDraw3( Shape )
				Pk = 0
				ke4.shape.info( Shape )
				local Filter = Filter or function( x, y )
					return x, y
				end
				Shape = ke4.shape.filtery( Shape,
					function( x, y )
						x, y = tonumber( x ), tonumber( y )		-- coordenadas "x" y "y" de cada punto
						Cx = c_shape							-- coordenada "x" del centro de la shape
						Cy = m_shape							-- coordenada "y" del centro de la shape
						Do = ke4.math.distance( x, y )			-- distancia del punto al origen
						Dc = ke4.math.distance( Cx, Cy, x, y )	-- distancia del punto al centro de la shape
						Ao = ke4.math.angle( x, y )				-- ángulo del origen al punto
						Ac = ke4.math.angle( Cx, Cy, x, y )		-- ángulo del centro al punto
						Pn = n_points							-- cantidad total de puntos en la shape
						Pk = Pk + 1								-- contador de los puntos de la shape
						Mx = (y - miny ) / h_shape				-- módulo de varianza respecto a "x", Mx = [0, 1]
						My = (x - minx ) / w_shape				-- módulo de varianza respecto a "y", My = [0, 1]
						Mp = (Pk - 1) / (Pn - 1)				-- módulo de varianza respecto a los puntos, Mp = [0, 1]
						return Filter( x, y )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end,
			
			filter2 = function( Shape, Filter, Split )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local Split = Split or 2 * ratio
				local Filter = Filter or function( x, y )
					return x, y
				end
				local Shape = ke4.shape.ASSDraw3( Shape )
				Shape = ke4.shape.split( Shape, Split )
				Shape = ke4.shape.flatten( Shape, Split )
				Pk = 0
				ke4.shape.info( Shape )
				Shape = ke4.shape.filtery( Shape,
					function( x, y )
						x, y = tonumber( x ), tonumber( y )		-- coordenadas "x" y "y" de cada punto
						Cx = c_shape							-- coordenada "x" del centro de la shape
						Cy = m_shape							-- coordenada "y" del centro de la shape
						Do = ke4.math.distance( x, y )			-- distancia del punto al origen
						Dc = ke4.math.distance( Cx, Cy, x, y )	-- distancia del punto al centro de la shape
						Ao = ke4.math.angle( x, y )				-- ángulo del origen al punto
						Ac = ke4.math.angle( Cx, Cy, x, y )		-- ángulo del centro al punto
						Pn = n_points							-- cantidad total de puntos en la shape
						Pk = Pk + 1								-- contador de los puntos de la shape
						Mx = (y - miny ) / h_shape				-- módulo de varianza respecto a "x", Mx = [0, 1]
						My = (x - minx ) / w_shape				-- módulo de varianza respecto a "y", My = [0, 1]
						Mp = (Pk - 1) / (Pn - 1)				-- módulo de varianza respecto a los puntos, Mp = [0, 1]
						return Filter( x, y )
						--las variables se deben llamar en los filtros usando el _G.
					end
				) --{\p1}!_G.ke4.shape.filter2( _G.ke4.text.to_shape( line.styleref, line.text_stripped ), function( x, y ) x = x + _G.ke4.math.Rcs( 2 ) y = y + _G.ke4.math.Rcs( 2 ) return x, y end, 3 )!
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.filter2( "m 50 0 b 22 0 0 22 0 50 b 0 78 22 100 50 100 b 78 100 100 78 100 50 b 100 22 78 0 50 0 ", function( x, y ) x = x + _G.ke4.math.Rcs( 2 ) y = y + _G.ke4.math.Rcs( 2 ) return x, y end, 1 )!
		
			filter3 = function( Shape, Split, ... )
				local Shape = ke4.shape.ASSDraw3( Shape )
				local filters = { ... }
				if type( ... ) == "table" then
					filters = ...
				end
				if #filters == 0 then
					filters[ 1 ] = function( x, y )
						return x, y
					end
				end
				if Split
					and Split ~= 0 then
					local Split3 = abs( Split )
					Shape = ke4.shape.split( Shape, Split3 )
					Shape = ke4.shape.flatten( Shape, Split3 )
				end
				ke4.shape.info( Shape )
				for i = 1, #filters do
					if type( filters[ i ] ) == "table"
						or type( filters[ i ] ) == "string" then
						local do_shapefx, mode_fx = nil, nil
						if type( filters[ i ] ) == "table" then
							do_shapefx, mode_fx = filters[ i ][ 1 ], filters[ i ][ 2 ]
						else
							do_shapefx, mode_fx = filters[ i ], nil
						end
						Shape = ke4.shape.do_shape( Shape, do_shapefx, mode_fx )
					else
						if type( filters[ i ] ) ~= "function" then
							filters[ i ] = function( x, y )
								return x, y
							end
						end
						Pk = 0
						Shape = ke4.shape.filtery( Shape,
							function( x, y )
								x, y = tonumber( x ), tonumber( y )		-- coordenadas "x" y "y" de cada punto
								Cx = c_shape							-- coordenada "x" del centro de la shape
								Cy = m_shape							-- coordenada "y" del centro de la shape
								Do = ke4.math.distance( x, y )			-- distancia del punto al origen
								Dc = ke4.math.distance( Cx, Cy, x, y )	-- distancia del punto al centro de la shape
								Ao = ke4.math.angle( x, y )				-- ángulo del origen al punto
								Ac = ke4.math.angle( Cx, Cy, x, y )		-- ángulo del centro al punto
								Pn = n_points							-- cantidad total de puntos en la shape
								Pk = Pk + 1								-- contador de los puntos de la shape
								Mx = (y - miny ) / h_shape				-- módulo de varianza respecto a "x", Mx = [0, 1]
								My = (x - minx ) / w_shape				-- módulo de varianza respecto a "y", My = [0, 1]
								Mp = (Pk - 1) / (Pn - 1)				-- módulo de varianza respecto a los puntos, Mp = [0, 1]
								return filters[ i ]( x, y )
							end
						)
					end
					ke4.shape.info( Shape )
				end
				return ke4.shape.ASSDraw3( Shape )
			end,
			
			length = function( Shape, parts )
				--retorna la longitud de la Shape ingresada o una tabla
				--con los valores de cada una de las secciones de la Shape
				local Shape = ke4.shape.ASSDraw3( Shape )
				local shape_parts, shape_segments, shape_length = { }, { }, 0
				for c in Shape:gmatch( "%S+" ) do
					table.insert( shape_parts, c )
				end
				for k = 1, #shape_parts do
					local segments, n = { }, 0
					if shape_parts[ k ] == "l"
						or shape_parts[ k ] == "b" then
						n = ( shape_parts[ k ] == "l" ) and 5 or 9
						for i = 1, n do
							segments[ i ] = tonumber( shape_parts[ k - 3 + i ] )
						end
						table.remove( segments, 3 )
						table.insert( shape_segments, segments )
					end
				end
				if parts then
					return shape_segments
				end
				for i = 1, #shape_segments do
					shape_length = shape_length + ke4.math.length_bezier( shape_segments[ i ] )
				end
				return shape_length
			end, --!_G.ke4.shape.length( _G.ke4.shape.circle )!
		
			width = function( Shape, Height )
				--retorna el ancho de la Shape ingresada
				local Shape = ke4.shape.ASSDraw3( Shape )
				local shape_parts, shape_px, shape_py = { }, { }, { }
				for c in Shape:gmatch( "%-?%d+[%.%d]*" ) do
					table.insert( shape_parts, tonumber( c ) )
				end
				for i = 1, #shape_parts / 2 do
					shape_px[ i ] = shape_parts[ 2 * i - 1 ]
					shape_py[ i ] = shape_parts[ 2 * i - 0 ]
				end
				local shape_width, shape_height = 0, 0
				if #shape_parts > 0 then
					shape_width  = ke4.table.op( shape_px, "rank" )
					shape_height = ke4.table.op( shape_py, "rank" )
				end
				if Height == "height" then
					return shape_height
				end
				return shape_width
			end, --!_G.ke4.shape.width( _G.ke4.shape.pentagon )!
			
			height = function( Shape )
				--retorna la altura de la Shape ingresada
				return ke4.shape.width( Shape, "height" )
			end, --!_G.ke4.shape.height( _G.ke4.shape.pentagon )!
			
			rotate = function( Shape, Angle, org_x, org_y )	-- in z axis
				--rota la Shape respecto al eje "z" con un punto de origen predeterminado
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Ang = Angle or 0
				local cx = org_x or 0
				local cy = org_y or 0
				if cx == "center" then
					ke4.shape.info( Shape )
					cx = c_shape
					cy = m_shape
				end
				Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)",
					function( x, y )
						local x, y = tonumber( x ), tonumber( y )
						local new_ang = ke4.math.angle( cx, cy, x, y )
						local new_rad = ke4.math.distance( cx, cy, x, y )
						x = cx + ke4.math.polar( new_ang + Ang, new_rad, "x" )
						y = cy + ke4.math.polar( new_ang + Ang, new_rad, "y" )
						return format( "%s %s", x, y )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end, --!_G.ke4.shape.rotate( _G.ke4.shape.rectangle, 15 )!
			
			reflect = function( Shape, Axis, Relative )
				--hace un reflejo de la Shape respecto a alguno de los 2 ejes, o a la recta y = x
				--o respecto a las rectas x = Relative o y = Relative
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Reltv = Relative or 0
				Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)",
					function( x, y )
						local x, y = tonumber( x ), tonumber( y )
						if Axis == "x" then
							y = Reltv - y
						elseif Axis == "y"
							or Axis == nil then
							x = Reltv - x
						else
							x = -Reltv - x
							y = -Reltv - y
						end
						return format( "%s %s", x, y )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.reflect( _G.ke4.shape.star )!

			oblique = function( Shape, Pixel, Axis )
				--le aplica un fx tipo "cursiva" a la Shape ingresada
				local Shape = ke4.shape.ASSDraw3( Shape )
				ke4.shape.info( Shape )
				local pxl1, pxl2 = Pixel, 0
				if type( Pixel ) == "table" then
					pxl1 = Pixel[ 1 ]
					pxl2 = Pixel[ 2 ] or 0
				end
				Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)",
					function( x, y )
						local x, y = tonumber( x ), tonumber( y )
						if Axis == "x"
							or Axis == nil then
							x = x + pxl1 * (y - miny) / h_shape
						elseif Axis == "y" then
							y = y + pxl1 * (x - minx) / w_shape
						else
							x = x + pxl1 * (y - miny) / h_shape
							y = y + pxl2 * (x - minx) / w_shape
						end
						return format( "%s %s", x, y )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.oblique( _G.ke4.shape.circle, 40, "x" )!

			to_bezier = function( Shape )
				--convierte las secciones "line" de la Shape, en "bezier"
				local Shape = ke4.shape.ASSDraw3( Shape )
				for i = 1, 2 do
					Shape = Shape:gsub( "(%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+l%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*)",
						function( seg_line )
							local coor = { }
							for c in seg_line:gmatch( "%-?%d+[%.%d]*" ) do
								table.insert( coor, tonumber( c ) )
							end
							local x1, y1, x4, y4 = coor[ 1 ], coor[ 2 ], coor[ 3 ], coor[ 4 ]
							local Ang = ke4.math.angle( x1, y1, x4, y4 )
							local Rad = ke4.math.distance( x1, y1, x4, y4 )
							local x2, x3 = x1 + ke4.math.polar( Ang, (1 / 3) * Rad, "x" ), x1 + ke4.math.polar( Ang, (2 / 3) * Rad, "x" )
							local y2, y3 = y1 + ke4.math.polar( Ang, (1 / 3) * Rad, "y" ), y1 + ke4.math.polar( Ang, (2 / 3) * Rad, "y" )
							return format( "%s %s b %s %s %s %s %s %s", x1, y1, x2, y2, x3, y3, x4, y4 )
						end
					)
				end
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.to_bezier( _G.ke4.shape.rectangle )!
			
			reverse = function( Shape )
				--revierte el sentido en que fue dibujada la Shape
				local Shape = ke4.shape.ASSDraw3( Shape )
				local shape1, shape2, shape3 = { }, { }, { }
				local index, shape_x, shape_y, shape_inv = { }, { }, { }, ""
				for c in Shape:gmatch( "%S+" ) do
					table.insert( shape1, c )
					table.insert( shape2, c )
				end
				shape1, shape2 = ke4.table.reverse( shape1 ), ke4.table.reverse( shape2 )
				for k = 1, #shape1 do
					if shape1[ k ] == "m"
						or shape1[ k ] == "l"
						or shape1[ k ] == "b" then
						table.insert( index, k )
					end
				end
				for k = 1, #shape1 do
					if shape1[ k ] == "m"
						or shape1[ k ] == "l"
						or shape1[ k ] == "b" then
						table.remove( shape1, k )
					end
				end
				for k = 1, #shape1 / 2 do
					shape_x[ k ] = tonumber( shape1[ 2 * k - 0 ] )
					shape_y[ k ] = tonumber( shape1[ 2 * k - 1 ] )
				end
				for k = 1, #shape1 do
					if k % 2 == 1 then
						shape3[ k ] = shape_x[ (k + 1) / 2 ]
					else
						shape3[ k ] = shape_y[ (k + 0) / 2 ]
					end
				end
				for k = 1, #index do
					if shape2[ index[ k ] ] ~= "b" then
						table.insert( shape3, index[ k ], shape2[ index[ k ] ] )
					else
						table.insert( shape3, index[ k ] - 4, shape2[ index[ k ] ] )
					end
				end
				for k = 1, #shape3 do
					shape_inv = shape_inv .. shape3[ k ] .. " "
				end
				shape_inv = shape_inv:sub( 1, -3 )
				shape_inv = "m " .. shape_inv
				return ke4.shape.ASSDraw3( shape_inv )
			end, --{\p1}!_G.ke4.shape.reverse( _G.ke4.shape.rectangle )!

			origin = function( Shape )
				--posiciona a la Shape en su ubicación por default (cuadrante IV del AssDraw3)
				local Shape = ke4.shape.ASSDraw3( Shape )
				ke4.shape.info( Shape )
				Shape = ke4.shape.displace( Shape, -minx, -miny )
				return ke4.shape.ASSDraw3( Shape )
			end,
			
			displace = function( Shape, Dx, Dy )
				--desplaza la Shape a las coordenadas seleccionadas
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Dx = Dx or 0
				local Dy = Dy or 0
				Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)", 
					function( x, y )
						local x, y = tonumber( x ), tonumber( y )
						return format( "%s %s", x + Dx, y + Dy )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.displace( _G.ke4.shape.circle, 20, 10 )!
			
			incenter = function( Shape )
				--Desplaza la Shape respecto a su centro, al punto P = (0, 0)
				local Shape = ke4.shape.origin( Shape )
				Shape = ke4.shape.displace( Shape, -ke4.shape.width( Shape ) / 2, -ke4.shape.height( Shape ) / 2)
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.incenter( _G.ke4.shape.rectangle )!
			
			centerpos = function( Shape, CenterX, CenterY )
				--Desplaza la Shape respecto a su centro, al punto P = ( CenterX, CenterY )
				local CenterX = CenterX or 0
				local CenterY = CenterY or 0
				local Shape = ke4.shape.displace( ke4.shape.incenter( Shape ), CenterX, CenterY )
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.centerpos( _G.ke4.shape.rectangle )!
			
			firstpos = function( Shape, pos_x, pos_y )
				--Desplaza la Shape respecto a su primer punto, al punto P = ( pos_x, pos_y )
				local Shape = ke4.shape.ASSDraw3( Shape )
				local first_x = pos_x or 0
				local first_y = pos_y or 0
				local first_p = { }
				if Shape:match( "m%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]* " ) then
					first_p.x = tonumber( Shape:match( "m%s+(%-?%d+[%.%d]*)%s+%-?%d+[%.%d]* " ) )
					first_p.y = tonumber( Shape:match( "m%s+%-?%d+[%.%d]*%s+(%-?%d+[%.%d]*) " ) )
				end
				Shape = ke4.shape.displace( Shape, first_x - first_p.x, first_y - first_p.y )
				return ke4.shape.ASSDraw3( Shape )
			end, --{\p1}!_G.ke4.shape.firstpos( _G.ke4.shape.circle )!
			
			ratio = function( Shape, Ratiox, Ratioy, Mode )
				--modifica el tamaño de la Shape respecto a una proporción (Ratio)
				local Mode = Mode or 0
				local Shape = ke4.shape.ASSDraw3( Shape )
				ke4.shape.info( Shape )
				local shpx1, shpx2 = minx, maxx
				local shpy1, shpy2 = miny, maxy
				local shp_w, shp_h = w_shape, h_shape
				local Rx, Ry
				if Ratiox then
					Rx = Ratiox
					if type( Ratiox ) == "table" then
						Rx = Ratiox[ 1 ] / ke4.shape.width( Shape )
					end
				else
					Rx = 1
				end
				if Ratioy then
					Ry = Ratioy
					if type( Ratioy ) == "table" then
						Ry = Ratioy[ 1 ] / ke4.shape.height( Shape )
						if Ratiox == nil then
							Rx = Ry
						end
					end
				else
					Ry = Rx
				end
				Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)", 
					function( x, y )
						--Mode = 0, respecto al punto (0, 0)
						local x, y = tonumber( x ), tonumber( y )
						return format( "%s %s", x * Rx, y * Ry )
					end
				)
				--------------------------------------------------------------------------------
				--desplaza la shape_fx respecto a las 9 posiciones notables de la Shape original
				if Mode == 1 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx1, shpy2 - ke4.shape.height( Shape ) )
				elseif Mode == 2 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx1 + 0.5 * shp_w - 0.5 * ke4.shape.width( Shape ), shpy2 - ke4.shape.height( Shape ) )
				elseif Mode == 3 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx2 - ke4.shape.width( Shape ), shpy2 - ke4.shape.height( Shape ) )
				elseif Mode == 4 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx1, shpy1 + 0.5 * shp_h - 0.5 * ke4.shape.height( Shape ) )
				elseif Mode == 5 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx1 + 0.5 * shp_w - 0.5 * ke4.shape.width( Shape ), shpy1 + 0.5 * shp_h - 0.5 * ke4.shape.height( Shape ) )
				elseif Mode == 6 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx2 - ke4.shape.width( Shape ), shpy1 + 0.5 * shp_h - 0.5 * ke4.shape.height( Shape ) )
				elseif Mode == 7 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx1, shpy1 )
				elseif Mode == 8 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx1 + 0.5 * shp_w - 0.5 * ke4.shape.width( Shape ), shpy1 )
				elseif Mode == 9 then
					return ke4.shape.displace( ke4.shape.origin( Shape ), shpx2 - ke4.shape.width( Shape ), shpy1 )
				end
				--------------------------------------------------------------------------------
				return Shape --> Mode = 0
			end, --{\p1}!_G.ke4.shape.ratio( _G.ke4.shape.circle, 1, 2 )!
			
			size = function( Shape, SizeX, SizeY, Mode )
				--modifica el tamaño de la Shape respecto a valores determinados
				--si SizeX es una tabla, SizeX[ 1 ] se sumará al ancho de la Shape
				--si SizeY es una tabla, SizeY[ 1 ] se sumará al alto de la Shape
				local Mode = Mode or 0
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Szx = SizeX or ke4.shape.width( Shape )
				local Szy = SizeY or Szx
				if type( Szx ) == "table" then
					Szx = ke4.shape.width( Shape ) + Szx[ 1 ]
				end
				if type( Szy ) == "table" then
					Szy = ke4.shape.height( Shape ) + Szy[ 1 ]
				end
				if Szx == 0 then
					--la dimensión en "x" se modifica proporcionalmente según cómo se modifique en "y"
					Shape = ke4.shape.ratio( Shape, nil, { Szy }, Mode )
				elseif Szy == 0 then
					--la dimensión en "y" se modifica proporcionalmente según cómo se modifique en "x"
					Shape = ke4.shape.ratio( Shape, { Szx }, nil, Mode )
				else
					Shape = ke4.shape.ratio( Shape, Szx / ke4.shape.width( Shape ), Szy / ke4.shape.height( Shape ), Mode )
				end
				return Shape
			end, --{\p1}!_G.ke4.shape.size( _G.ke4.shape.rectangle, 120, 45 )!

			divide = function( Shape, Mark )
				-- retorna una tabla con las shapes que conforman una shape
				-- si se quiere, retorna cada una de esas shapes con un marco
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Shapes, mark = { }, ""
				ke4.shape.info( Shape )
				if Mark == "default"
					or type( Mark ) == "number" then
					mark = format( "m %s %s m %s %s ", maxx, maxy, minx, miny )
				elseif type( Mark ) == "table" then
					mark = format( "m %s %s m %s %s ", Mark[ 3 ], Mark[ 4 ], Mark[ 1 ], Mark[ 2 ] )
				end
				for c in Shape:gmatch( "m%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*[%-?%d%.lb ]*" ) do
					table.insert( Shapes, mark .. c )
				end
				return Shapes
			end, --!_G.ke4.table.view( _G.ke4.shape.divide( "m 50 53 l 23 66 l 20 96 l 46 84 m 50 53 l 54 84 l 81 97 l 78 66 " ) )!
			
			to_shape = function( Table_points )
				--concatena una tabla con los valores de una Shape, para volverla en "string" nuevamente
				local Shape = ""
				for i = 1, #Table_points do
					Shape = Shape .. Table_points[ i ] .. " "
				end
				return Shape
			end,
			
			retire = function( Shape, Index_1, Index_2 )
				--retira el o los segmentos de la shape que se indiquen
				local Shape, coor = ke4.shape.ASSDraw3( Shape ), { }
				local I_1 = Index_1
				local I_2 = Index_2 or I_1
				for c in Shape:gmatch( "[mlb]^* %-?%d+[%.%d]* [%-%.%d ]*" ) do
					table.insert( coor, c )
				end
				return ke4.shape.to_shape( ke4.table.retire( coor, I_1, I_2 ) )
			end,
		
			array = function( Shape, Loops, Angles_or_mode, Dxy )
				--genera múltiples arreglos "array" de una o más shapes ingresadas
				local Loops = Loops or 6
				local An_mo = Angles_or_mode or 0
				local disxy = Dxy or 0
				local loop1 = Loops
				local loop2 = 1
				if type( Loops ) == "table" then
					loop1 = Loops[ 1 ]
					loop2 = Loops[ 2 ] or loop1
				end
				local angle_array = An_mo
				if type( angle_array ) ~= "number"
					and type( angle_array ) ~= "table" then
					angle_array = 0
				end
				local angle_shape = 0
				if type( An_mo ) == "table" then
					angle_array = An_mo[ 1 ]
					angle_shape = An_mo[ 2 ] or 0
				end
				local distance_x, radius_ini = disxy, disxy
				if type( disxy ) == "string" then
					distance_x, radius_ini = 0, 0
				end
				local distance_y, radius_fin = distance_x, radius_ini
				local arco, angle_ini, distance_r = 360, 0, 0
				if type( disxy ) == "table" then
					distance_x = disxy[ 1 ]
					distance_y = disxy[ 2 ] or distance_x
					radius_ini = disxy[ 1 ]
					radius_fin = disxy[ 2 ] or radius_ini
					arco	   = disxy[ 3 ] or 360
					angle_ini  = disxy[ 4 ] or 0
					distance_r = disxy[ 5 ] or 0
				end
				local Shapes, shape_radial, shape_radial_fx, shape_array_fx, shape_lineal_fx = { }, "", "", "", ""
				local shape_left, idx, shape_rectangular_maxwidths
				local shape_array, shape_rectangular, shape_lineal = { }, { }, { }
				local shape_rectangular_widths = { }
				local widths, shp_lefts = { [ 0 ] = 0 }, { [ 0 ] = 0 }
				local heights, shp_tops = { [ 0 ] = 0 }, { [ 0 ] = 0 }
				if type( Shape ) == "table" then
					Shapes = Shape
				elseif Shape == "random" then
					--!maxloop( 84 )!{\p1\an5\pos(!55 + 106 * (_G.ke4.math.i( j, 12 )["1-->A"] - 1)!, !-50 + 102 * _G.ke4.math.i( j, 12 )["N,n"]!)}!_G.ke4.shape.array( "random" )!
					local shp0 = {
						[ 1 ] = format( "m 0 0 l %s %s l %s %s l %s %s l %s %s l 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 )
						),
						[ 2 ] = format( "m 0 0 b %s %s %s %s %s %s l %s %s b %s %s %s %s 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ),
							ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 )
						),
						[ 3 ] = format( "m 0 0 b %s %s %s %s %s %s b %s %s %s %s %s %s l 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ),
							ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 )
						),
						[ 4 ] = format( "m 0 0 l %s %s l %s %s b %s %s %s %s %s %s l 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 )
						),
						[ 5 ] = format( "m 0 0 l %s %s b %s %s %s %s %s %s l %s %s l 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 )
						),
						[ 6 ] = format( "m 0 0 b %s %s %s %s %s %s l %s %s b %s %s %s %s %s %s l 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ),
							ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 )
						),
						[ 6 ] = format( "m 0 0 b %s %s %s %s %s %s l %s %s l %s %s b %s %s %s %s 50 0 ", 
							ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ),
							ke4.math.Rcs( 30 ), ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ),
							ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 )
						),
						[ 7 ] = format( "m 0 0 l %s 0 b %s %s %s %s %s %s l %s %s b %s %s %s %s 50 0 ", 
							ke4.math.Rc( 20, 40 ), ke4.math.Rc( 25, 45 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 0, 20 ), ke4.math.Rcs( 30 ),
							ke4.math.Rc( 20, 40 ), ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 ), ke4.math.Rc( 20, 40 ),
							ke4.math.Rcs( 40 ), ke4.math.Rc( 10, 30 ), ke4.math.Rcs( 30 )
						),
					}
					local shp1 = shp0[ R( #shp0 ) ]
					local shp2 = ke4.shape.reverse( ke4.shape.reflect( shp1, "x" ) ):gsub( "m 50 0 ", "" )
					Shapes = { ke4.shape.ratio( shp1 .. shp2, { ke4.math.Rc( 27, 50 ) } ) }
					loop1, loop2 = 2 * ke4.math.R( 3, 4 ), 1
					radius_ini = 50 - ke4.shape.width( Shapes[ 1 ] )
					radius_fin = radius_ini
					An_mo = "radial2"
				else
					Shapes = { Shape }
				end
				if An_mo == "shape"
					or An_mo == "shape1" then
					--[[
					local configs_pos = math.shape( disxy, nil, nil, loop1 * loop2 )
					local shp_array = ""
					for i = 1, #configs_pos do
						shp_array = shp_array .. shape.displace(
							shape.rotate( shape.incenter( Shapes[ (i - 1) % #Shapes + 1 ] ), configs_pos[ i ][ 3 ] ),
							configs_pos[ i ][ 1 ], configs_pos[ i ][ 2 ]
						)
						-- la opción de que la shape rote o no
					end --shape.array( shape.size( shape.rectangle, 15, 45 ), 20, "shape", shape.circle )
					return shape.origin( shp_array )
					--]]
				elseif An_mo == "shape2"
					or An_mo == "shape3" then
					--[[
					local configs_pos = math.shape( disxy, nil, nil, loop1 * loop2 )
					local shp_array = ""
					for i = 1, #configs_pos do
						if i % 2 == 0 then
							Shapes[ (i - 1) % #Shapes + 1 ] = shape.reverse( Shapes[ (i - 1) % #Shapes + 1 ] )
						end
						shp_array = shp_array .. shape.displace(
							shape.rotate( shape.incenter( Shapes[ (i - 1) % #Shapes + 1 ] ), configs_pos[ i ][ 3 ] ),
							configs_pos[ i ][ 1 ], configs_pos[ i ][ 2 ]
						)
					end --shape.array( shape.size( shape.rectangle, 15, 45 ), 24, "shape2", shape.circle )
					return shape.origin( shp_array )
					--]]
				elseif An_mo == "radial3" then
					local radio_array, arcox
					for i = 1, #Shapes do
						shape_array[ i ] = ""
						widths[ i ] = ke4.shape.width( Shapes[ i ] ) + distance_r
						shp_lefts[ i ] = shp_lefts[ i - 1 ] + widths[ i - 1 ]
						for k = 1, loop2 do
							shape_array[ i ] = shape_array[ i ] .. ke4.shape.displace( ke4.shape.origin( Shapes[ i ] ),
								(k - 1) * widths[ i ],
								-0.5 * ke4.shape.height( Shapes[ i ] )
							)
						end
					end
					for i = 1, loop1 do
						idx = (i - 1) % #Shapes + 1
						radio_array = radius_ini + (radius_fin - radius_ini) * (i - 1) / (loop1 - 1)
						if loop1 == 1 then
							radio_array = radius_ini
						end
						arcox = (i - 1) * arco / loop1
						if loop1 > 1
							and arco % 360 ~= 0 then
							arcox = (i - 1) * arco / (loop1 - 1)
						end
						shape_radial_fx = shape_radial_fx .. ke4.shape.rotate( ke4.shape.displace( shape_array[ idx ],
							radio_array ), angle_ini + arcox
						)
					end
					return ke4.shape.origin( shape_radial_fx )	--radial3
				elseif An_mo == "radial"
					or An_mo == "radial1"
					or An_mo == "radial2" then
					local radio_array, arcox
					for i = 1, loop2 do
						idx = (i - 1) % #Shapes + 1
						shape_array[ i ] = ke4.shape.displace( ke4.shape.origin( Shapes[ idx ] ), 0, -0.5 * ke4.shape.height( Shapes[ idx ] ) )
						widths[ i ] = ke4.shape.width( Shapes[ idx ] ) + distance_r
						shp_lefts[ i ] = shp_lefts[ i - 1 ] + widths[ i - 1 ]
						shape_radial = shape_radial .. ke4.shape.displace( shape_array[ i ], shp_lefts[ i ] )
					end
					local shape_reverse
					for i = 1, loop1 do
						radio_array = radius_ini + (radius_fin - radius_ini) * (i - 1) / (loop1 - 1)
						if loop1 == 1 then
							radio_array = radius_ini
						end
						arcox = (i - 1) * arco / loop1
						if loop1 > 1
							and arco % 360 ~= 0 then
							arcox = (i - 1) * arco / (loop1 - 1)
						end
						shape_reverse = shape_radial
						if i % 2 == 0
							and An_mo == "radial2" then
							shape_reverse = ke4.shape.reverse( shape_radial )
						end
						shape_radial_fx = shape_radial_fx .. ke4.shape.rotate( ke4.shape.displace( shape_reverse, radio_array ),
							angle_ini + arcox
						)
					end
					return ke4.shape.origin( shape_radial_fx )	--radial1, radial2
				elseif An_mo == "array" then
					for i = 1, loop2 do
						idx = (i - 1) % #Shapes + 1
						Shapes[ idx ] = ke4.shape.origin( Shapes[ idx ] )
						heights[ i ] = ke4.shape.height( Shapes[ idx ] ) + distance_y
						shp_tops[ i ] = shp_tops[ i - 1 ] + heights[ i - 1 ]
						shape_rectangular[ i ] = ""
						for k = 1, loop1 do
							shape_rectangular[ i ] = shape_rectangular[ i ] .. ke4.shape.displace( Shapes[ idx ], 
								(k - 1) * ke4.shape.width( Shapes[ idx ] ) + distance_x * (k - 1)--ke4.math.i( k )[ "0<->11" ]
							)
						end
						shape_rectangular_widths[ i ] = ke4.shape.width( shape_rectangular[ i ] )
						shape_rectangular[ i ] = ke4.shape.displace( shape_rectangular[ i ], 0, shp_tops[ i ] )
					end
					shape_rectangular_maxwidths = math.max( unpack( shape_rectangular_widths ) )
					for i = 1, loop2 do
						shape_rectangular[ i ] = ke4.shape.displace( shape_rectangular[ i ],
							(shape_rectangular_maxwidths - ke4.shape.width( shape_rectangular[ i ] )) / 2
						)
						shape_array_fx = shape_array_fx .. shape_rectangular[ i ]
					end
					return shape_array_fx
				end
				for i = 1, loop1 do
					idx = (i - 1) % #Shapes + 1
					shape_lineal[ i ] = ke4.shape.origin( ke4.shape.rotate( Shapes[ idx ], angle_shape - angle_array ) )
					shape_lineal[ i ] = ke4.shape.displace( shape_lineal[ i ], 0, -0.5 * ke4.shape.height( shape_lineal[ i ] ) )
					widths[ i ] = ke4.shape.width( shape_lineal[ i ] ) + distance_x
					shp_lefts[ i ] = shp_lefts[ i - 1 ] + widths[ i - 1 ]
					shape_lineal_fx = shape_lineal_fx .. ke4.shape.displace( shape_lineal[ i ], shp_lefts[ i ] )
				end
				return ke4.shape.origin( ke4.shape.rotate( shape_lineal_fx, angle_array ) )
			end,
			
			trajectory = function( Loop_t, distance_nim, distance_max )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local Loop_t = ceil( 3 * abs( Loop_t or 15 ) )--ceil( Loop_t or linefx[ ii ].duration / 720 )
				local Dr_min = distance_nim or 10 * ratio
				local Dr_max = distance_max or 20 * ratio
				local n, dist, Ang, px, py = 3 * (Loop_t + 1), ke4.math.R( Dr_min, Dr_max ), { }, { }, { }
				Ang[ -1 ] = ke4.math.R( 17 ) * 17
				Ang[ 0 ] = Ang[ -1 ] - 180
				px[ -1 ] = ke4.math.polar( Ang[ -1 ], dist, "x" )
				py[ -1 ] = ke4.math.polar( Ang[ -1 ], dist, "y" )
				px[ 0 ] = px[ -1 ] + ke4.math.polar( Ang[ 0 ], dist, "x" )
				py[ 0 ] = py[ -1 ] + ke4.math.polar( Ang[ 0 ], dist, "y" )
				for i = 1, n do
					dist = ke4.math.R( Dr_min, Dr_max )
					local Val = (i - 1) % 3 + 1
					if Val == 1 then
						Ang[ i ] = Ang[ i - 1 ]
					elseif Val == 2 then
						Ang[ i ] = Ang[ i - 1 ] + 90 * (-1) ^ ke4.math.R( 2 ) + ke4.math.Rs( 0, 50, 5 )
					elseif Val == 3 then
						Ang[ i ] = Ang[ i - 1 ] + 90 * (-1) ^ i - ke4.math.Rs( 0, 50, 5 )
					end
					px[ i ] = px[ i - 1 ] + ke4.math.polar( Ang[ i ], dist, "x" )
					py[ i ] = py[ i - 1 ] + ke4.math.polar( Ang[ i ], dist, "y" )
				end
				local tags, k = format( "m %s %s ", px[ 0 ], py[ 0 ] ), 0
				while k <= Loop_t do
					tags = tags .. format( "b %s %s %s %s %s %s ", 
						px[ 3 * k + 1 ], py[ 3 * k + 1 ], px[ 3 * k + 2 ], py[ 3 * k + 2 ], px[ 3 * k + 3 ], py[ 3 * k  + 3 ]
					)
					k = k + 1
				end
				return ke4.shape.ASSDraw3( tags )
			end, --{\p1}!_G.ke4.shape.trajectory( )!
			
			Ltrajectory = function( length_total, length_curve, height_curve )
				-- Curve in Line Trajectory
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local ratio_y = height_curve or 40 * ratio
				local lengthC = length_curve or xres / 4
				local lengthT = length_total or 400--xres - val_center
				local Loop_Lt = ke4.math.round( lengthT / lengthC )
				local loops, Rand, px, py = 3 * Loop_Lt, ratio_y, { }, { }
				local Ang, Rad
				for i = 0, loops, 3 do
					px[ i ] = i * lengthC / 3
					py[ i ] = ke4.math.Rs( Rand / 4 )
				end
				for i = 1, loops + 1, 3 do
					px[ i ] = px[ i - 1 ] + ke4.math.Rs( 0.6 * lengthC )
					py[ i ] = ke4.math.Rs( 0.7 * Rand, Rand )
				end
				for i = 2, loops, 3 do
					Ang = ke4.math.angle( px[ i + 2 ], py[ i + 2 ], px[ i + 1 ], py[ i + 1 ] )
					Rad = ke4.math.distance( px[ i + 2 ], py[ i + 2 ], px[ i + 1 ], py[ i + 1 ] )
					px[ i ] = px[ i + 1 ] + ke4.math.polar( Ang, Rad, "x" )
					py[ i ] = py[ i + 1 ] + ke4.math.polar( Ang, Rad, "y" )
				end
				local k, tags = 0, "m 0 0 "
				while k <= Loop_Lt - 1 do
					tags = tags .. format( "b %s %s %s %s %s %s ", 
						px[ 3 * k + 1 ], py[ 3 * k + 1 ], px[ 3 * k + 2 ], py[ 3 * k + 2 ], px[ 3 * k + 3 ], py[ 3 * k + 3 ]
					)
					k = k + 1
				end
				return ke4.shape.ASSDraw3( tags )
			end, --!_G.ke4.shape.Ltrajectory( )!
			
			Ctrajectory = function( Loop_Ct, radius_min, radius_max )
				-- Circle Trajectory
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local R_max = radius_max or xres / 25
				local R_min = radius_min or xres / 40
				local loops = ceil( 3 * abs( Loop_Ct or 15 ) )--ceil( 3 * (Loop_Ct or linefx[ ii ].duration / 720) )
				local px, py = { }, { }
				local Ang, Rad
				for i = -3, loops, 3 do
					Ang = ke4.math.R( 0, 360, 10 )
					Rad = ke4.math.R( R_min, R_max )
					px[ i ] = ke4.math.polar( Ang, Rad, "x" )
					py[ i ] = ke4.math.polar( Ang, Rad, "y" )
				end
				for i = -2, loops + 1, 3 do
					Ang = ke4.math.R( 0, 360, 18 )
					Rad = ke4.math.R( R_min, 2 * R_max )
					px[ i ] = px[ i - 1 ] + ke4.math.polar( Ang, Rad, "x" )
					py[ i ] = py[ i - 1 ] + ke4.math.polar( Ang, Rad, "y" )
				end
				for i = -1, loops, 3 do
					Ang = ke4.math.angle( px[ i + 2 ], py[ i + 2 ], px[ i + 1 ], py[ i + 1 ] )
					Rad = ke4.math.R( R_min / 2, 2.5 * R_max )
					px[i] = px[ i + 1 ] + ke4.math.polar( Ang, Rad, "x" )
					py[i] = py[ i + 1 ] + ke4.math.polar( Ang, Rad, "y" )
				end
				local k, tags = 0, format( "m 0 0 b %s %s %s %s %s %s ", px[ -2 ], py[ -2 ], px[ -1 ], py[ -1 ], px[ 0 ], py[ 0 ] )
				while k <= loops / 3 - 2 do
					tags = tags .. format( "b %s %s %s %s %s %s ",
						px[ 3 * k + 1 ], py[ 3 * k + 1 ], px[ 3 * k + 2 ], py[ 3 * k + 2 ], px[ 3 * k + 3 ], py[ 3 * k + 3 ]
					)
					k = k + 1
				end
				return ke4.shape.ASSDraw3( tags )
			end, --!_G.ke4.shape.Ctrajectory( )!
			
			Rtrajectory = function( Loop_Rt, radius_min, radius_max )
				-- Random Trajectory
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local R_max = radius_max or xres / 25
				local R_min = radius_min or xres / 40
				local loops = ceil( 3 * abs( Loop_Rt or 15 ) )--ceil( 3 * (Loop_Rt or linefx[ ii ].duration / 720) )
				local px, py = { }, { }
				local Ang, Rad
				for i = -3, loops, 3 do
					Ang = ke4.math.R( 0, 360, 10 )
					Rad = ke4.math.R( R_min, R_max )
					px[ i ] = ke4.math.polar( Ang, Rad, "x" )
					py[ i ] = ke4.math.polar( Ang, Rad, "y" )
				end
				for i = -2, loops + 1, 3 do
					Ang = ke4.math.R( 0, 360, 18 )
					Rad = ke4.math.R( R_min, 2 * R_max )
					px[ i ] = px[ i - 1 ] + ke4.math.polar( Ang, Rad, "x" )
					py[ i ] = py[ i - 1 ] + ke4.math.polar( Ang, Rad, "y" )
				end
				for i = -1, loops, 3 do
					Ang = ke4.math.R( 0, 360, 12 )
					Rad = ke4.math.R( R_min / 2, 2.5 * R_max )
					px[ i ] = px[ i + 1 ] + ke4.math.polar( Ang, Rad, "x" )
					py[ i ] = py[ i + 1 ] + ke4.math.polar( Ang, Rad, "y" )
				end
				local k, tags = 0, format( "m 0 0 b %s %s %s %s %s %s ", px[ -2 ], py[ -2 ], px[ -1 ], py[ -1 ], px[ 0 ], py[ 0 ] )
				while k <= loops / 3 - 2 do
					tags = tags .. format( "b %s %s %s %s %s %s ",
						px[ 3 * k + 1 ], py[ 3 * k + 1 ], px[ 3 * k + 2 ], py[ 3 * k + 2 ], px[ 3 * k + 3 ], py[ 3 * k + 3 ]
					)
					k = k + 1
				end
				return ke4.shape.ASSDraw3( tags )
			end, --!_G.ke4.shape.Rtrajectory( )!
			
			Strajectory = function( Loops_St, Radius )
				-- Segment Line Trajectory
				local Radius = Radius or 45--0.75 * val_height
				local loops = ceil( abs( Loops_St or 12 ) )--ceil( Loops_St or linefx[ ii ].duration / 820 )
				local angles = { [ 0 ] = ke4.math.R( 0, 360, 10 ) }
				for i = 1, loops do
					angles[ i ] = ke4.math.R( angles[ i - 1 ] + 110, angles[ i - 1 ] + 250 )
				end
				local tags, Rand = "m 0 0 ", 0
				for i = 1, loops do
					Rand = ke4.math.R( 0.7 * Radius, Radius )
					tags = format( "%sl %s %s ", tags, ke4.math.polar( angles[ i ], Rand, "x" ), ke4.math.polar( angles[ i ], Rand, "y" ) )
				end
				return ke4.shape.ASSDraw3( tags )
			end, --!_G.ke4.shape.Strajectory( )!
			
			multi1 = function( Size_shape, Px )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local Shape = recall.shape_multi1
				--if j == 1 then
					local i = 1
					local Px = Px or 4 * ratio
					local shpw, Pxi, Px1 = 0, 0, 0
					if Size_shape == "default"
						or Size_shape == nil then
						Size_shape = 60--math.max( val_width, val_height )
					end
					if type( Px ) == "number" then
						Px = { Px }
					end
					if type( Px ) == "table" then
						Px1 = Px[ 1 ]
					elseif type( Px ) == "function" then
						Px1 = Px( 1 )
					end
					shpw = Px1
					Shape = format( "m 0 0 l 0 %s l %s %s l %s 0 l 0 0 ", Px1, Px1, Px1, Px1 )
					while 2 * shpw < Size_shape do
						if type( Px ) == "table" then
							Pxi = Px[ i % #Px + 1 ]
						elseif type( Px ) == "function" then
							Pxi = Px( i )
						end
						shpw = shpw + ((type( Pxi ) == "table") and Pxi[ 1 ] or Pxi)
						if type( Pxi ) == "number" then
							Shape = Shape .. format( "m %s %s l %s %s l %s %s l %s %s l %s %s l %s %s l %s %s l %s %s l %s %s l %s %s ", 
								-shpw + Px1, -shpw + Px1, -shpw + Px1, shpw, shpw, shpw, shpw, -shpw + Px1, -shpw + Px1, -shpw + Px1,
								-shpw + Px1 + Pxi, -shpw + Px1 + Pxi, shpw - Pxi, -shpw + Px1 + Pxi, shpw - Pxi, shpw - Pxi,
								-shpw + Px1 + Pxi, shpw - Pxi, -shpw + Px1 + Pxi, -shpw + Px1 + Pxi
							)
						end --mod: january 02nd 2019
						i = i + 1
					end
					Shape = remember( "shape_multi1", ke4.shape.origin( Shape ) )
				--end
				return ke4.shape.ASSDraw3( Shape ) --{\p1}!_G.ke4.shape.multi1( 100, { 10, { 4 } } )!
			end, --retorna shapes cuadradas concéntricas
			
			multi2 = function( Width, Height, Pixel )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local Shape = recall.shape_multi2
				--if j == 1 then	
					local Pixel = Pixel or 6 * ratio
					local Width = Width or 120--val_width
					local Height = Height or 50--val_height
					if type( Pixel ) == "number" then
						Pixel = { Pixel }
					end
					local dimen = { x = Width, y = Height }
					if Height > Width then
						dimen = { x = Height, y = Width }
					end
					local i, Pxl_i = 1, 0
					local Shp2 = format( "m 0 0 l 0 %s l %s 0 l 0 0 l 0 0 ", Pixel[ 1 ], Pixel[ 1 ] )
					local dim_y = Pixel[ 1 ]
					while dim_y < dimen.y do
						if type( Pixel ) == "table" then
							Pxl_i = Pixel[ i % #Pixel + 1 ]
						end
						dim_y = dim_y + ((type( Pxl_i ) == "table") and Pxl_i[ 1 ] or Pxl_i)
						if type( Pxl_i ) == "number" then
							Shp2 = Shp2 .. format( "m 0 %s l 0 %s l %s 0 l %s 0 l 0 %s ", dim_y - Pxl_i, dim_y, dim_y, dim_y - Pxl_i, dim_y - Pxl_i )
						end
						i = i + 1
					end
					local dim_x, shp_y = dim_y, dim_y
					while dim_x < dimen.x do
						if type( Pixel ) == "table" then
							Pxl_i = Pixel[ i % #Pixel + 1 ]
						end
						dim_x = dim_x + ((type( Pxl_i ) == "table") and Pxl_i[ 1 ] or Pxl_i)
						if type( Pxl_i ) == "number" then
							Shp2 = Shp2 .. format( "m %s %s l %s %s l %s 0 l %s 0 l %s %s ",
								dim_x - shp_y - Pxl_i, shp_y, dim_x - shp_y, shp_y, dim_x, dim_x - Pxl_i, dim_x - shp_y - Pxl_i, shp_y
							)
						end
						i = i + 1
					end
					local shp_x = 0
					while shp_x < shp_y do
						if type( Pixel ) == "table" then
							Pxl_i = Pixel[ i % #Pixel + 1 ]
						end
						shp_x = shp_x + ((type( Pxl_i ) == "table") and Pxl_i[ 1 ] or Pxl_i)
						if type( Pxl_i ) == "number"
							and shp_x <= shp_y then
							Shp2 = Shp2 .. format( "m %s %s l %s %s l %s %s l %s %s l %s %s ",
								dim_x - shp_y + shp_x - Pxl_i, shp_y, dim_x - shp_y + shp_x, shp_y,
								dim_x, shp_x, dim_x, shp_x - Pxl_i, dim_x - shp_y + shp_x - Pxl_i, shp_y
							)
						end
						i = i + 1
					end
					if shp_x - ((type( Pxl_i ) == "table") and Pxl_i[ 1 ] or Pxl_i) < shp_y
						and type( Pxl_i ) == "number" then
						shp_x = shp_x - Pxl_i
						Shp2 = Shp2 .. format( "m %s %s l %s %s l %s %s l %s %s ",
							dim_x - shp_y + shp_x, shp_y, dim_x, shp_y, dim_x, shp_x, dim_x - shp_y + shp_x, shp_y
						)
					end
					if Height > Width then
						Shp2 = ke4.shape.reflect( ke4.shape.rotate( Shp2, -90 ) )
					end
					Shape = remember( "shape_multi2", Shp2 )
				--end
				return ke4.shape.ASSDraw3( Shape ) --{\p1}!_G.ke4.shape.multi2( 36, 94, { 10, { 2 }, 5, { 3 } } )!
			end, --crea shapes diagonales dentro de un rectángulo con medidas dadas
			
			multi3 = function( Size, Bord, Shape )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local Shape3 = recall.shape_multi3
				--if j == 1 then
					local Shape = Shape or ke4.shape.circle
					local i, Shpfx1, Shpfx2 = 1, "", ""
					local Bord = Bord or 4 * ratio
					if type( Bord ) == "number" then
						Bord = { Bord }
					end
					local Size_max, Brd_i = Bord[ 1 ], 0
					if Size == "default"
						or Size == nil then
						Size = 64--(val_width ^ 2 + val_height ^ 2) ^ 0.5
					end
					Shape3 = ke4.shape.incenter( ke4.shape.size( Shape, Bord[ 1 ] ) )
					while Size_max <= Size do
						if type( Bord ) == "table" then
							Brd_i = Bord[ i % #Bord + 1 ]
						end
						Size_max = Size_max + 2 * ((type( Brd_i ) == "table") and Brd_i[ 1 ] or Brd_i)
						if type( Brd_i ) == "number" then
							Shpfx1 = ke4.shape.incenter( ke4.shape.size( Shape, Size_max ) )
							Shpfx2 = ke4.shape.incenter( ke4.shape.reverse( ke4.shape.size( Shape, Size_max - 2 * Brd_i ) ):gsub( "m", "l", 1 ) )
							Shape3 = Shape3 .. Shpfx1 .. Shpfx2
						end
						i = i + 1
					end
					Shape3 = remember( "shape_multi3", ke4.shape.origin( Shape3 ) )
				--end --{\p1}!_G.ke4.shape.multi3( 100, { 8, { 5 } } )!
				return ke4.shape.ASSDraw3( Shape3 )
			end, --si no se pone "Shape", retorna círculos concéntricos, o shapes concéntricas de la que se haya ingresado
			
			multi4 = function( Size, Loop1, Loop2, n )
				local Shapefx = recall.Shpfx
				--if j == 1 then
					local Sizer
					local Shape = ""
					local n = ke4.math.round( abs( n or 25 ) )
					if Size == "default" then
						Sizer = 86--(val_width ^ 2 + val_height ^ 2) ^ 0.5
					else
						Sizer = Size
					end
					Sizer = ke4.math.round( Sizer or 64--[[(val_width ^ 2 + val_height ^ 2) ^ 0.5]] )
					Sizer = 2 * ceil( Sizer / 2 )
					local Loop1 = ke4.math.round( abs( Loop1 or 6 ) )
					if Loop1 < 3 then
						Loop1 = 3
					end
					local Pn = Sizer / 2
					local Loop2 = ke4.math.round( Loop2 or 1 )
					if Loop2 <= 0 then
						Loop2 = 1
					end
					local Px = ke4.math.round( 0.5 * Sizer / Loop2 )
					local function multi40( Size40, Loop40, Px40 )
						local Size40 = 2 * ceil( Size40 / 2 )
						if Px40 >= Size40 / 2 then
							Px40 = Size40 / 2
						end
						local Angle = 360 / Loop40
						local Shapes = { }
						local Shape40 = ""
						for i = 1, ke4.math.round( 360 / Angle ) do
							Shapes[ i ] = format( "m %s %s l %s %s l %s %s l %s %s ", 
								ke4.math.polar( Angle * (i - 0), Size40 / 2 - Px40, "x" ),	ke4.math.polar( Angle * (i - 0), Size40 / 2 - Px40, "y" ),
								ke4.math.polar( Angle * (i - 0), Size40 / 2, "x" ),			ke4.math.polar( Angle * (i - 0), Size40 / 2, "y" ),
								ke4.math.polar( Angle * (i - 1), Size40 / 2, "x" ),			ke4.math.polar( Angle * (i - 1), Size40 / 2, "y" ),
								ke4.math.polar( Angle * (i - 1), Size40 / 2 - Px40, "x" ),	ke4.math.polar( Angle * (i - 1), Size40 / 2 - Px40, "y" )
							)
							Shape40 = Shape40 .. Shapes[ i ]
						end
						return Shape40
					end
					local i = 0
					while Pn > 0
						and i < n do
						Shape = Shape .. multi40( Pn * 2, Loop1, Px )
						Pn = Pn - Px
						i = i + 1
					end
					Shapefx = remember( "Shpfx", ( Loop1 % 2 == 1 )
						and ke4.shape.origin( ke4.shape.rotate( Shape, ((-1) ^ ((Loop1 - 1) / 2)) * 90 / Loop1 ) )
						or  ke4.shape.origin( Shape )
					)
				--end
				return Shapefx --{\p1}!_G.ke4.shape.multi4( 100, 6, 4, 3 )!
			end, --retorna un polígono regular de Loop1 lados, con un arreglo de Loop2. n es la cantidad de arreglos tomados en cuenta
			
			multi5 = function( Shapes, Width, Height, Dxy )
				local Shape = recall.shape_multi5
				--if j == 1 then
					local widths, heights, ShapeT, dis_xy = { }, { }, { }, { }
					local max_W, max_H
					if type( Shapes ) == "table" then
						for i = 1, #Shapes do
							widths[ i ] = ke4.shape.width( Shapes[ i ] )
							heights[ i ] = ke4.shape.height( Shapes[ i ] )
						end
						max_W = math.max( unpack( widths ) )
						max_H = math.max( unpack( heights ) )
						for i = 1, #Shapes do
							ShapeT[ i ] = ""
							for k = 1, #Shapes do
								ShapeT[ i ] = ShapeT[ i ] .. ke4.shape.displace(
									ke4.shape.incenter( Shapes[ (k - i) % #Shapes + 1 ] ), (k - 1) * max_W, (i - #Shapes) * max_H
								)
							end
						end
						Shape = ke4.shape.origin( ke4.table.op( ShapeT, "concat" ) )
					else
						Shape = Shapes or ke4.shape.size( shape.rectangle, 8 * ratio )
						Shape = ke4.shape.origin( Shape )
					end
					local Height = Height or 50--val_height
					local Width = Width or 64--val_width
					if Dxy == nil then
						dis_xy = { 0, 0 }
					elseif type( Dxy ) == "number" then
						dis_xy = { Dxy, 0 }
					else
						dis_xy = Dxy
					end
					local length_H = ceil( Width / (ke4.shape.width( Shape ) + dis_xy[ 1 ]) )
					local length_V = ceil( Height / (ke4.shape.height( Shape ) + dis_xy[ 2 ]) )
					Shape = remember( "shape_multi5", ke4.shape.array( Shape, { length_H, length_V }, "array", dis_xy ) )
				--end
				return ke4.shape.ASSDraw3( Shape ) --{\p1}!_G.ke4.shape.multi5( )!
			end, --retorna un arreglo matricial rectangular de las shapes ingresadas
			
			multi6 = function( Size, Bord, Part )
				local Size = Size or 104
				local Bord = Bord or 4
				local Part = Part or 20
				local Shape = recall.shape_multi6
				--if j == 1 then
					Part = 4 * ceil( Part / 4 )
					local Wdt = ke4.math.round( (Size - Bord) / (Part / 4), 2 )
					local sh_top, sh_right, sh_bottom, sh_left = "", "", "", ""
					for i = 1, Part / 4 do
						sh_top = sh_top .. format( "m %s 0 l %s 0 l %s %s l %s %s ",
							Wdt * (i - 1), Wdt * i, Wdt * i, Bord, Wdt * (i - 1), Bord
						)
					end
					for i = 1, Part / 4 do
						sh_right = sh_right .. format( "m %s %s l %s %s l %s %s l %s %s ",
							Size, Wdt * (i - 1), Size, Wdt * i, Size - Bord, Wdt * i, Size - Bord, Wdt * (i - 1)
						)
					end
					for i = 1, Part / 4 do
						sh_bottom = sh_bottom .. format( "m %s %s l %s %s l %s %s l %s %s ",
							Size - Wdt * (i - 1), Size, Size - Wdt * i, Size, Size - Wdt * i, Size - Bord, Size - Wdt * (i - 1), Size - Bord
						)
					end
					for i = 1, Part / 4 do
						sh_left = sh_left .. format( "m 0 %s l 0 %s l %s %s l %s %s ",
							Size - Wdt * (i - 1), Size - Wdt * i, Bord, Size - Wdt * i, Bord, Size - Wdt * (i - 1)
						)
					end
					Shape = remember( "shape_multi6", sh_top .. sh_right .. sh_bottom .. sh_left )
				--end
				return ke4.shape.ASSDraw3( Shape ) --{\p1}!_G.ke4.shape.multi6( )!
			end, --retorna el perímetro de un cuadrado formado de rectángulos individuales
			
			multi7 = function( Part, Radius )
				local Part = Part or 12
				local Radius = Radius or 50
				local Shape, Angle, Ang_B, Ratio = recall.shape_multi7
				--if j == 1 then
					Part = ke4.math.round( abs( Part ) )
					if Part < 2 then
						Part = 2
					end
					Angle = 360 / Part
					Ang_B = Angle * 0.295927
					Ratio = 1 / cos( rad( Ang_B ) ) --sin( deg( Angle / 2 ) )
					Shape = ""
					if type( Radius ) == "number" then
						for i = 1, Part do
							Shape = Shape .. format( "m 0 0 l %s %s b %s %s %s %s %s %s l 0 0 ",
								ke4.math.polar( Angle * ( i - 1 ), Radius, "x" ),					ke4.math.polar( Angle * ( i - 1 ), Radius, "y" ),
								ke4.math.polar( Angle * ( i - 1 ) + Ang_B, Radius * Ratio, "x" ),	ke4.math.polar( Angle * ( i - 1 ) + Ang_B, Radius * Ratio, "y" ),
								ke4.math.polar( Angle *  i - Ang_B, Radius * Ratio, "x" ),			ke4.math.polar( Angle *  i - Ang_B, Radius * Ratio, "y" ),
								ke4.math.polar( Angle *  i, Radius, "x" ),							ke4.math.polar( Angle *  i, Radius, "y" )
							)
						end
					else --type( Radius ) == "table"
						for i = 1, #Radius - 1 do
							for k = 1, Part do
								Shape = Shape .. format( "m %s %s b %s %s %s %s %s %s l %s %s b %s %s %s %s %s %s l %s %s ",
									ke4.math.polar( Angle * ( k - 1 ), Radius[ i + 1 ], "x" ),					ke4.math.polar( Angle * ( k - 1 ), Radius[ i + 1 ], "y" ),
									ke4.math.polar( Angle * ( k - 1 ) + Ang_B, Radius[ i + 1 ] * Ratio, "x" ),	ke4.math.polar( Angle * ( k - 1 ) + Ang_B, Radius[ i + 1 ] * Ratio, "y" ),
									ke4.math.polar( Angle *  k - Ang_B, Radius[ i + 1 ] * Ratio, "x" ),			ke4.math.polar( Angle *  k - Ang_B, Radius[ i + 1 ] * Ratio, "y" ),
									ke4.math.polar( Angle *  k, Radius[ i + 1 ], "x" ),							ke4.math.polar( Angle *  k, Radius[ i + 1 ], "y" ),
									ke4.math.polar( Angle *  k, Radius[ i ], "x" ),								ke4.math.polar( Angle *  k, Radius[ i ], "y" ),
									ke4.math.polar( Angle *  k - Ang_B, Radius[ i ] * Ratio, "x" ),				ke4.math.polar( Angle *  k - Ang_B, Radius[ i ] * Ratio, "y" ),
									ke4.math.polar( Angle * ( k - 1 ) + Ang_B, Radius[ i ] * Ratio, "x" ),		ke4.math.polar( Angle * ( k - 1 ) + Ang_B, Radius[ i ] * Ratio, "y" ),
									ke4.math.polar( Angle * ( k - 1 ), Radius[ i ], "x" ),						ke4.math.polar( Angle * ( k - 1 ), Radius[ i ], "y" ),
									ke4.math.polar( Angle * ( k - 1 ), Radius[ i + 1 ], "x" ),					ke4.math.polar( Angle * ( k - 1 ), Radius[ i + 1 ], "y" )
								)
							end
						end
					end
					Shape = remember( "shape_multi7", ke4.shape.origin( Shape ) )
				--end
				return ke4.shape.ASSDraw3( Shape ) --{\p1}!_G.ke4.shape.multi7( 12, { 20, 40, 60 } )!
			end, --retorna un círculo o el perímetro de un círculo hecho con segmentos individuales
			
			multi8 = function( Shape, Size_ini, Size_fin, Loop )
				local Shape = Shape or shape.rectangle
				Shape = ke4.shape.origin( Shape )
				local Size_i = Size_ini or ke4.shape.width( Shape )
				local Size_f = Size_fin or ke4.shape.width( Shape ) * 0.5
				local Loop_s = Loop or 8
				Size_i = abs( ceil( Size_i ) )
				Size_f = abs( ceil( Size_f ) )
				Loop_s = abs( ceil( Loop_s ) )
				if Loop_s < 2 then
					Loop_s = 2
				end
				local Size_max = math.max( Size_i, Size_f )
				local Size_min = math.min( Size_i, Size_f )
				local Shp_init = ke4.shape.size( Shape, Size_max, 0 )
				-- hace que lo más complicado de la función se ejecute sólo una vez -----------------------
				local Shp_mul8 = recall.shape8
				--if j == 1 then
					Shp_mul8 = ""
					local Shp_cent, Shp_midd = 0.5 * ke4.shape.width( Shp_init ), 0.5 * ke4.shape.height( Shp_init )
					for i = 1, Loop_s do
						Shp_mul8 = Shp_mul8 .. ke4.shape.centerpos( ke4.shape.size( Shp_init, Size_max - (Size_max - Size_min) * (i - 1) / (Loop_s - 1), 0 ), Shp_cent, Shp_midd )
					end
					Shp_mul8 = remember( "shape8", Shp_mul8 )
				--end
				-------------------------------------------------------------------------------------------
				return Shp_mul8 --{\p1}!_G.ke4.shape.multi8( _G.ke4.shape.rectangle, 100, 10, 10 )!
			end, --retorna shapes concéntricas desde una tamaño inicial hasta un tamaño final
			
			multi9 = function( Shape, Loop, Tags, Vertical )
				local Shape = Shape or shape.rectangle
				local Loop_s = Loop or 8
				if Loop_s < 2 then
					Loop_s = 2
				end
				local Tags_s = Tags
				if Tags == nil then
					Tags_s = { }
					for i = 1, Loop_s do
						Tags_s[ i ] = format( "{\\1c%s}", ke4.random.color( nil, 82 ) )
					end
				end
				if ke4.table.type( Tags_s ) == "table" then
					Tags_s = ke4.table.concat4( Tags_s )
				end --december 06th 2019
				local tag_N = "{"
				if Vertical then
					tag_N = "{\\p0}\\N{\\p1"
				end
				local Shps_9 = "{\\p1}"
				if type( Shape ) == "string" then
					Shape = ke4.shape.origin( Shape )
					for i = 1, Loop_s do
						Shps_9 = Shps_9 .. tag_N .. Tags_s[ (i - 1) % #Tags_s + 1 ] .. "}" .. Shape
					end
				elseif type( Shape ) == "function" then
					for i = 1, Loop_s do
						Shps_9 = Shps_9 .. tag_N .. Tags_s[ (i - 1) % #Tags_s + 1 ] .. "}" .. ke4.shape.origin( Shape( i, Loop_s ) )
					end
					--[[
					my_filter = function( ... )
						local tbl = { ... }
						local i = tbl[ 1 ]
						local n = tbl[ 2 ]
						local mod = (i - 1) / (n - 1)
						return string.format( "m 0 0 l 0 50 l 0 %s l 4 %s l 4 %s l 0 %s ",
							40 + 10 * math.sin( math.pi * mod ), 40 + 10 * math.sin( math.pi * mod ),
							10 - 10 * math.sin( math.pi * mod ), 10 - 10 * math.sin( math.pi * mod )
						)
					end;
					Loop = math.ceil( (line.width + 160) / 4 );
					tags = table.ipol( { "&HFF&", "&H5A&", "&HFF&" }, Loop, "\\1a" )
					!_G.ke4.shape.multi9( my_filter, Loop, tags )!
					--]]
				end
				-- elimina el primer salto de línea \N de la shape
				Shps_9 = Shps_9:gsub( "{\\p0}\\N{\\p1", "{", 1 )
				--------------------------------------------------
				return Shps_9
			end, --crea una shape formada por una secuencia de shapes en una única línea de fx
		
			bord = function( Shape, Bord, Size )
				local Shape = ke4.shape.ASSDraw3( Shape )
				ke4.shape.info( Shape )
				local P_first, Size_x, Size_y, Shape_1, Shape_2, Shape_B = { }, w_shape, h_shape, "", "", ""
				P_first.x = tonumber( Shape:match( "m (%-?%d+[%.%d]*) %-?%d+[%.%d]* " ) )
				P_first.y = tonumber( Shape:match( "m %-?%d+[%.%d]* (%-?%d+[%.%d]*) " ) )
				local Size = Size or { [ 1 ] = w_shape, [ 2 ] = h_shape }
				local Bord = Bord or 4
				if type( Size ) == "table" then
					Size_x, Size_y = Size[ 1 ], Size[ 2 ]
				elseif type( Size ) == "number" then
					Size_x, Size_y = Size, Size
				end
				Bord = abs( Bord )
				if  Bord > floor( math.min( Size_x, Size_y ) / 2 ) - 1 then
					Bord = floor( math.min( Size_x, Size_y ) / 2 ) - 1
				end
				Shape_1 = ke4.shape.incenter( ke4.shape.size( Shape, Size_x, Size_y ) )
				Shape_2 = ke4.shape.incenter( ke4.shape.size( ke4.shape.reverse( Shape ), Size_x - 2 * Bord, Size_y - 2 * Bord ) )
				Shape_2 = "l" .. Shape_2:sub( 2, -1 )
				Shape_B = Shape_1 .. Shape_2
				return ke4.shape.firstpos( Shape_B, P_first.x, P_first.y )
			end, --{\p1}!_G.ke4.shape.bord( _G.ke4.shape.circle, 8, 120 )!
		
			equality = function( Shape1, Shape2 )
				local segm1, segm2, Shapefx1, Shapefx2 = { }, { }, { }, { }
				for c in Shape1:gmatch( "[blm]* %-?%d+[%.%d]* [%-%.%d ]*" ) do
					table.insert( segm1, c )
				end
				for c in Shape2:gmatch( "[blm]* %-?%d+[%.%d]* [%-%.%d ]*" ) do
					table.insert( segm2, c )
				end
				local difference = abs( #segm1 - #segm2 )
				if difference == 0 then
					return { Shape1, Shape2 }
				end
				local n, N, segm
				if #segm1 < #segm2 then
					n, N = ceil( difference / #segm1 ), ceil( #segm1 / difference )
					for i = 1, #segm1 do
						if i % N == 0 then
							segm = ""
							for k = 1, N + 1 do
								segm = segm .. segm1[ i ]
							end
							Shapefx1[ i ] = segm
						else
							Shapefx1[ i ] = segm1[ i ]
						end
					end
					Shapefx1 = ke4.table.op( Shapefx1, "concat" )
					Shapefx2 = Shape2
				else
					n, N = ceil( difference / #segm2 ), ceil( #segm2 / difference )
					for i = 1, #segm2 do
						if i % N == 0 then
							segm = ""
							for k = 1, N + 1 do
								segm = segm .. segm2[ i ]
							end
							Shapefx2[ i ] = segm
						else
							Shapefx2[ i ] = segm2[ i ]
						end
					end
					Shapefx1 = Shape1
					Shapefx2 = ke4.table.op( Shapefx2, "concat" )
				end
				return Shapefx1, Shapefx2
			end,
			
			morphism = function( Size, Shape1, Shape2, Accel )
				local Accel  = Accel or 1
				local Shapes = recall.shape_morphism
				--if j == 1 then
					local coor1, coor2 = { }, { }
					local k
					if Size < 2
						or Size == nil then
						Size = 4
					end
					Size = ke4.math.round( Size )
					for c in Shape1:gmatch( "%-?%d+[%.%d]*" ) do
						table.insert( coor1, tonumber( c ) )
					end
					for c in Shape2:gmatch( "%-?%d+[%.%d]*" ) do
						table.insert( coor2, tonumber( c ) )
					end
					Shapes = { }
					for i = 1, Size do
						k = 1
						Shapes[ i ] = Shape1:gsub( "%-?%d+[%.%d]*", 
							function( val )
								local val = tonumber( val )
								if coor2[ (k - 1) % #coor1 + 1 ]
									and coor1[ k ] then
									val = val + (coor2[ (k - 1) % #coor1 + 1 ] - coor1[ k ]) * ((i - 1) / (Size - 1)) ^ Accel
								else
									val = val + (coor1[ (k - 1) % #coor2 + 1 ] - coor2[ k ]) * (1 - (i - 1) / (Size - 1)) ^ Accel
								end
								k = k + 1
								return val
							end
						)
						Shapes[ i ] = ke4.shape.ASSDraw3( Shapes[ i ] )
					end
					Shapes = remember( "shape_morphism", Shapes )
				--end
				return Shapes --_G.ke4.shape.morphism( 6, _G.ke4.shape.to_bezier( _G.ke4.shape.rectangle ), _G.ke4.shape.circle )
			end, --retorna una tabla con la interpolación entre las dos shapes ingresadas
			
			to_pixels = function( Shape )
				local Shape = ke4.shape.ASSDraw3( Shape )
				ke4.shape.info( Shape )
				local pixel_datas, pixel = { }, { }
				local pixel_table = ke4.shape.to_pixels2( Shape )
				for i = 1, #pixel_table do
					pixel_datas[ i ] = { }
					for k, v in pairs( pixel_table[ i ] ) do
						table.insert( pixel_datas[ i ], v )
					end
				end
				for i = 1, #pixel_table do
					pixel[ i ] = { }
					pixel[ i ].x = -w_shape / 2 + pixel_datas[ i ][ 2 ]
					pixel[ i ].y = -h_shape / 2 + pixel_datas[ i ][ 1 ]
					pixel[ i ].a = ke4.alpha.val2ass( 255 - pixel_datas[ i ][ 3 ] )
				end
				return pixel
				--code once:	px = _G.ke4.shape.to_pixels( "m 0 0 l 0 50 l 50 50 l 50 0 " )
				--template:		!maxloop( #px )!{\pos(!$x + px[ j ].x!,!$y + px[ j ].y!)\alpha!px[ j ].a!\bord0\shad0\p1}m 0 0 l 0 1 l 1 1 l 1 0 
			end, --!_G.ke4.table.view( _G.ke4.shape.to_pixels( "m 0 0 l 0 10 l 10 10 l 10 0 " ) )!
			
			point = function( Shape, Pixel )
				--retorna una tabla con todos los puntos de la shape ingresada: { { px1, py1 }, { px2, py2 }, ... }
				local Pixel = Pixel or 2
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Points = { }
				local i = 1
				local ShapeR = ke4.shape.redraw( Shape, Pixel )
				ke4.shape.info( ShapeR )
				for pnt in ShapeR:gmatch( "%-?%d+[%.%d]* %-?%d+[%.%d]*" ) do
					Points[ i ] = {
						x = tonumber( pnt:match( "(%-?%d+[%.%d]*) %-?%d+[%.%d]*" ) ),
						y = tonumber( pnt:match( "%-?%d+[%.%d]* (%-?%d+[%.%d]*)" ) )
					}
					i = i + 1
				end
				return Points
			end, --!_G.ke4.table.view( _G.ke4.shape.point( _G.ke4.shape.circle ) )!
			
			bord_to_pixels = function( Shape, Pixel )
				local Shape = ke4.shape.ASSDraw3( Shape )
				local size_pixel = Pixel or 2
				ke4.shape.info( Shape )
				local points = ke4.shape.point( Shape, size_pixel )
				for i = 1, #points do
					points[ i ].x = points[ i ].x - 0.5 * w_shape
					points[ i ].y = points[ i ].y - 0.5 * h_shape
				end
				return points
				-- code once: points = _G.ke4.shape.bord_to_pixels( _G.ke4.shape.circle )
			end, --!maxloop( #points )!{\an5\pos(!$x + points[ j ].x!,!$y + points[ j ].y!)\bord0\shad0\p1}m 0 0 l 0 1 l 1 1 l 1 0 
			
			fxline = function( P1, P2, Radius )
				local Shape, P2x, P2y = ""
				if type( P2 ) == "table" then
					Shape = format( "m %s %s l %s %s ", P1[ 1 ], P1[ 2 ], P2[ 1 ], P2[ 2 ] )
				else
					P2x = P1[ 1 ] + ke4.math.polar( P2, Radius, "x" )
					P2y = P1[ 2 ] + ke4.math.polar( P2, Radius, "y" )
					Shape = format( "m %s %s l %s %s ", P1[ 1 ], P1[ 2 ], P2x, P2y )
				end
				return ke4.shape.ASSDraw3( Shape )
			end, --!_G.ke4.shape.fxline( { 300, 600 }, 45, 500 )!
			
			fxcircle = function( Shape )
				local ShapeC, Cpx, Cpy, Radius = ""
				if type( Shape ) == "string"
					and Shape ~= "" then
					local Shape = ke4.shape.ASSDraw3( Shape )
					Cpx, Cpy, Radius = ke4.math.circle( Shape )
					ShapeC = ke4.shape.displace( ke4.shape.incenter( ke4.shape.size( ke4.shape.circle, 2 * Radius ) ), Cpx, Cpy )
					ShapeC = format( "{\\an7\\pos(0,0)}%s", ShapeC )
				elseif type( Shape ) == "table" then
					ShapeC = ke4.shape.displace( ke4.shape.incenter( ke4.shape.size( ke4.shape.circle, 2 * Shape[ 3 ] ) ), Shape[ 1 ], Shape[ 2 ] )
					ShapeC = format( "{\\an7\\pos(0,0)}%s", ShapeC )
				end
				return ShapeC
			end, --{\p1}!_G.ke4.shape.fxcircle( "\\clip(m 416 440 l 450 220 758 228)" )!
			
			trim = function( Shape, Lines, Mark, Ratio )
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Mark = Mark or ""
				local Ratio = Ratio or 1
				ke4.shape.info( Shape)
				if Mark == 1 then
					Mark = format( "m %s %s m %s %s ", maxx, maxy, minx, miny )
				elseif type( Mark ) == "table" then
					Mark = format( " m %s %s m 0 0 ", Mark[ 1 ] * Ratio, Mark[ 2 ] * Ratio )
				end
				local trim_tbl = recall.trim_table
				--if j == 1 then
					trim_tbl = remember( "trim_table", { Shape } )
					local trim_n
					local function get_shapes( Shape, Line )
						local function get_var( Line )
							local x1, y1, x2, y2, m, b, ang
							if type( Line ) == "table" then
								x1, y1 = Line[ 1 ], Line[ 2 ]
								if #Line == 4 then -- tabla de 2 puntos
									x2, y2 = Line[ 3 ], Line[ 4 ]
								elseif #Line == 3 then
									if type( Line[ 3 ] ) == "number" then --tabla de punto y ángulo
										x2 = x1 + ke4.math.polar( Line[ 3 ], 1, "x" )
										y2 = y1 + ke4.math.polar( Line[ 3 ], 1, "y" )
									elseif type( Line[ 3 ] ) == "table" then --tabla de punto y pendiente
										x2 = x1 + 1
										y2 = y1 + Line[ 3 ][ 1 ]
									end
								else
									ang = 5 + ke4.math.R( 36 ) * 10
									x2 = x1 + ke4.math.polar( ang, 1, "x" )
									y2 = y1 + ke4.math.polar( ang, 1, "y" )
								end
							else
								local coor = { }
								for num in Line:gmatch( "%-?%d+[%.%d]*" ) do
									table.insert( coor, tonumber( num ) )
								end
								x1, y1, x2, y2 = coor[ 1 ], coor[ 2 ], coor[ 3 ], coor[ 4 ]
							end
							x1, y1, x2, y2 = x1 * Ratio, y1 * Ratio, x2 * Ratio, y2 * Ratio
							if x1 ~= x2 then
								m = (y2 - y1) / (x2 - x1)
								b = y1 - m * x1
							else
								m = x1
								b = nil
							end
							return m, b
						end
						local function get_point( Shape, Line )
							local shp_trim = { }
							local m, b = get_var( Line )
							if b then
								shp_trim[ 1 ] = Shape:gsub( "([mlb]*) (%-?%d+[%.%d]*) (%-?%d+[%.%d]*) ",
									function( c, x, y )
										if c and tonumber( y ) <= m * tonumber( x ) + b then
											if tonumber( y ) + 0.99 * Ratio >= m * tonumber( x ) + b then
												y = m * tonumber( x ) + b
											end
											return format( "%s %s %s ", c, x, y )
										end
										return ""
									end
								)
								shp_trim[ 2 ] = Shape:gsub( "([mlb]*) (%-?%d+[%.%d]*) (%-?%d+[%.%d]*) ",
									function( c, x, y )
										if c and tonumber( y ) >= m * tonumber( x ) + b then
											if tonumber( y ) + 0.99 * Ratio <= m * tonumber( x ) + b then
												y = m * tonumber( x ) + b
											end
											return format( "%s %s %s ", c, x, y )
										end
										return ""
									end
								)
							else
								shp_trim[ 1 ] = Shape:gsub( "([mlb]*) (%-?%d+[%.%d]*) (%-?%d+[%.%d]*) ",
									function( c, x, y )
										if c and tonumber( x ) >= m then
											if tonumber( x ) + 0.99 * Ratio <= m then
												x = m
											end
											return format( "%s %s %s ", c, x, y )
										end
										return ""
									end
								)
								shp_trim[ 2 ] = Shape:gsub( "([mlb]*) (%-?%d+[%.%d]*) (%-?%d+[%.%d]*) ",
									function( c, x, y )
										if c and tonumber( x ) <= m then
											if tonumber( x ) + 0.99 * Ratio >= m then
												x = m
											end
											return format( "%s %s %s ", c, x, y )
										end
										return ""
									end
								)
							end
							return shp_trim
						end
						local Line = Line or "m 0 0 l 1 0 "
						local trims = { }
						local shape_trims = { [ 1 ] = "", [ 2 ] = "" }
						local Shapes = ke4.shape.divide( Shape )
						for i = 1, #Shapes do
							Shapes[ i ] = ke4.shape.redraw( Shapes[ i ], Ratio )
							trims[ i ] = get_point( Shapes[ i ], Line )
						end
						for i = 1, #trims do
							trims[ i ][ 1 ] = "m" .. trims[ i ][ 1 ]:sub( 2, -1 )
							trims[ i ][ 2 ] = "m" .. trims[ i ][ 2 ]:sub( 2, -1 )
							if trims[ i ][ 1 ]:match( "m (%-?%d+[%.%d]* %-?%d+[%.%d]* )" ) then
								trims[ i ][ 1 ] = trims[ i ][ 1 ] .. trims[ i ][ 1 ]:match( "m (%-?%d+[%.%d]* %-?%d+[%.%d]* )" )
							end
							if trims[ i ][ 2 ]:match( "m (%-?%d+[%.%d]* %-?%d+[%.%d]* )" ) then
								trims[ i ][ 2 ] = trims[ i ][ 2 ] .. trims[ i ][ 2 ]:match( "m (%-?%d+[%.%d]* %-?%d+[%.%d]* )" )
							end
							if trims[ i ][ 1 ] == "m" then
								trims[ i ][ 1 ] = ""
							end
							if trims[ i ][ 2 ] == "m" then
								trims[ i ][ 2 ] = ""
							end
						end
						for i = 1, #trims do
							shape_trims[ 1 ] = shape_trims[ 1 ] .. trims[ i ][ 1 ]
							shape_trims[ 2 ] = shape_trims[ 2 ] .. trims[ i ][ 2 ]
						end
						shape_trims[ 1 ] = ke4.shape.reduce( ke4.shape.ASSDraw3( shape_trims[ 1 ] ), 5 )
						shape_trims[ 2 ] = ke4.shape.reduce( ke4.shape.ASSDraw3( shape_trims[ 2 ] ), 5 )
						for i, v in pairs( shape_trims ) do
							if v == "" then
								table.remove( shape_trims, i )
							end
						end
						return shape_trims
					end
					local trim_aux = { }
					for i = 1, #Lines do
						trim_n = #trim_tbl
						for k = 1, trim_n do
							trim_aux = ke4.table.inserttable( trim_aux, get_shapes( trim_tbl[ k ], Lines[ i ] ) )
						end
						trim_tbl = ke4.table.duplicate( trim_aux )
						trim_aux = { }
					end
					for i = 1, #trim_tbl do
						trim_tbl[ i ] = ke4.shape.ASSDraw3( Mark ) .. trim_tbl[ i ]
					end
				--end
				return trim_tbl
			end, --tbl = shape.trim( shape.rectangle, { "m 0 100 l 100 0 ", "m 0 0 l 100 100 ", { 0, 20, 100, 20 } } )

			reduce = function( Shape )
				local Shape = ke4.shape.ASSDraw3( Shape )
				local function get_reduce( Shape_red )
					Shape_red = Shape_red:gsub( "(%-?%d+[%.%d]*) (%-?%d+[%.%d]*) l (%-?%d+[%.%d]*) (%-?%d+[%.%d]*) l (%-?%d+[%.%d]*) (%-?%d+[%.%d]*)",
						function( x1, y1, x2, y2, x3, y3 )
							local angle1 = ke4.math.angle( x1, y1, x2, y2 )
							local angle2 = ke4.math.angle( x1, y1, x3, y3 )
							if ke4.math.round( angle1, 1 ) == ke4.math.round( angle2, 1 ) then
								return format( "%s %s l %s %s", x1, y1, x3, y3 )
							end
						end
					)
					return Shape_red
				end
				while Shape ~= get_reduce( Shape ) do
					Shape = get_reduce( Shape )
				end
				return Shape
				--shape.reduce( shape.redraw( shape.rectangle, 4 ) )
			end, --shape.reduce( "m 0 0 l 0 2 l 0 3 l 0 5 l 0 6 l 0 8 l 0 9 l 0 11 l 0 12 l 0 14 " )

			matrix = function( Shape, ... )
				local Matrixes = { ... }
				if #Matrixes == 0 then
					Matrixes[ 1 ] = {
						1,	0,	0,
						0,	1,	0,
						0,	0,	1
					}
				end
				for i = 1, #Matrixes do
					if #Matrixes[ i ] ~= 9 then
						error( "<<Error: shape.matrix>> Cada matriz debe ser de tamaño 3x3\n3x3 matrix expected", 2 )
					end
					for _, v in ipairs( Matrixes[ i ] ) do
						if type( v ) ~= "number" then
							error( "<<Error: shape.matrix>> Cada matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
				end
				local Matrix = ke4.math.matrix_mul2( unpack( Matrixes ) )
				local Shape = ke4.shape.ASSDraw3( Shape )
				Shape = Shape:gsub( "(%-?%d+[%.%d]*) (%-?%d+[%.%d]*)",
					function( x, y )
						local Points = { tonumber( x ), tonumber( y ), 1 } --> coordenadas homogéneas
						local Mtable = ke4.math.matrix_mul( Points, Matrix )
						x = Mtable[ 1 ]
						y = Mtable[ 2 ]
						return format( "%s %s", x, y )
					end
				)
				return ke4.shape.ASSDraw3( Shape )
			end,
			
			iso = function( Shape1, Shape2, Trim )
				local Trim = Trim or 8
				local Shape1 = ke4.shape.filter2( Shape1, nil, Trim )
				local Shape2 = ke4.shape.filter2( Shape2, nil, Trim )
				local Shapes1 = ke4.shape.divide( Shape1 )
				local Shapes2 = ke4.shape.divide( Shape2 )
				local function equ( Table1, Table2 )
					local table_max, table_min
					local orden, tables = { }, { }
					if #Table1 >= #Table2 then
						table_max = ke4.table.duplicate( Table1 )
						table_min = ke4.table.duplicate( Table2 )
						orden = { 2, 1 }
					else
						table_max = ke4.table.duplicate( Table2 )
						table_min = ke4.table.duplicate( Table1 )
						orden = { 1, 2 }
					end
					for i = 1, #table_max - #table_min do
						table.insert( table_min, table_min[ i ] )
					end
					tables = {
						[ 1 ] = table_min,
						[ 2 ] = table_max
					}
					return tables[ orden[ 1 ] ], tables[ orden[ 2 ] ]
				end
				local function equ_tramos( Tramo1, Tramo2 )
					local function tramos( Shape )
						segments = { }
						for c in Shape:gmatch( "[mlb]^* %-?%d+[%.%d]*[%-%d%. ]*" ) do
							table.insert( segments, c )
						end
						return segments
					end
					local segment_1 = tramos( Tramo1 )
					local segment_2 = tramos( Tramo2 )
					segment_1[ 1 ] = segment_1[ 1 ]:gsub( "m", "l" )
					segment_2[ 1 ] = segment_2[ 1 ]:gsub( "m", "l" )
					local segm_max, segm_min
					local orden, Shapes = { }, { }
					if #segment_1 >= #segment_2 then
						segm_max = segment_1
						segm_min = segment_2
						orden = { 2, 1 }
					else
						segm_max = segment_2
						segm_min = segment_1
						orden = { 1, 2 }
					end
					for i = 1, #segm_max - #segm_min do
						table.insert( segm_min, 2 * i - 1, segm_min[ 2 * i - 1 ] )
					end
					segm_min[ 1 ] = segm_min[ 1 ]:gsub( "l", "m" )
					segm_max[ 1 ] = segm_max[ 1 ]:gsub( "l", "m" )
					Shapes = {
						[ 1 ] = ke4.table.op( segm_min, "concat" ),
						[ 2 ] = ke4.table.op( segm_max, "concat" )
					}
					return Shapes[ orden[ 1 ] ], Shapes[ orden[ 2 ] ]
				end
				Shapes1, Shapes2 = equ( Shapes1, Shapes2 )
				for i = 1, #Shapes1 do
					Shapes1[ i ], Shapes2[ i ] = equ_tramos( Shapes1[ i ], Shapes2[ i ] )
				end
				return ke4.table.op( Shapes1, "concat" ), ke4.table.op( Shapes2, "concat" )
			end, --shape.iso( shape.circle, shape.rectangle )
			
			do_shape = function( Shape1, Shape2, Mode, Split )
				local Split = Split or 2
				local Shape1 = ke4.shape.ASSDraw3( Shape1 )
				local Shape2 = ke4.shape.ASSDraw3( ke4.shape.origin( Shape2 ) )
				local Shape1 = ke4.shape.filter2( Shape1, nil, Split )
				local Ratio = ke4.math.round( ke4.shape.width( Shape1 ) / ke4.shape.length( Shape2 ), 3 )
				local Mode = Mode or 1
				local Filter
				if type( Mode ) == "function" then
					Filter = Mode
				end
				if ke4.shape.length( Shape2 ) < ke4.shape.width( Shape1 ) then
					Filter = function( x, y )
						return x, y
					end
				else
					if Mode == 1 then		-- centrado [conserva la longitud de shape1]
						Filter = function( x, y )
							return (1 - Ratio) / 2 + Ratio * x, y
						end
					elseif Mode == 2 then	-- de inicio a fin [conserva la longitud de shape1]
						Filter = function( x, y )
							return Ratio * x, y
						end
					elseif Mode == 3 then	-- de fin a inicio [conserva la longitud de shape1]
						Filter = function( x, y )
							return 1 - Ratio + Ratio * x, y
						end
					elseif Mode == 4 then	-- justificado a lo largo [modifica la longitud de Shape1 hasta igualar la de Shape2]
						Filter = function( x, y )
							return x, y
						end
					end
				end
				Pk = 0
				ke4.shape.info( Shape1 )
				local shape_do_shape = ke4.shape.ASSDraw3( ke4.shape.glue( Shape1, Shape2,
					function( x, y )
						Cx = c_shape							-- coordenada en "x" del centro de la shape
						Cy = m_shape							-- coordenada en "y" del centro de la shape
						Do = ke4.math.distance( x, y )			-- distancia del punto al origen
						Dc = ke4.math.distance( Cx, Cy, x, y )	-- distancia del punto al centro de la shape
						Ao = ke4.math.angle( x, y )				-- ángulo del origen al punto
						Ac = ke4.math.angle( Cx, Cy, x, y )		-- ángulo del centro al punto
						Pn = n_points							-- cantidad total de puntos en la shape
						Pk = Pk + 1								-- contador de los puntos de la shape
						Mx = (y - miny ) / h_shape				-- módulo de varianza respecto a "x", Mx = [0, 1]
						My = (x - minx ) / w_shape				-- módulo de varianza respecto a "y", My = [0, 1]
						Mp = (Pk - 1) / (Pn - 1)				-- módulo de varianza respecto a los puntos, Mp = [0, 1]
						return Filter( x, y )
					end
				) )
				return shape_do_shape
			end, --{\p1}!_G.ke4.shape.do_shape( _G.ke4.shape.size( _G.ke4.shape.rectangle, 280, 20 ), _G.ke4.shape.circle, 1, 4 )!
			
			to_outline = function( Shape, Bord )
				local Bord = (Bord or 1.02) / 2
				local Shape = ke4.shape.ASSDraw3( Shape )
				Shape = ke4.shape.flatten( Shape, 2 )
				local Shape_outline = ke4.shape.to_outline2( Shape, Bord )
				return ke4.shape.ASSDraw3( Shape_outline )
			end, --{\p1}!_G.ke4.shape.to_outline( _G.ke4.shape.rectangle )!
			
			deformed = function( Shape, Deformed, Pixel, Axis )
				local Axis = Axis or "x"
				local Pixel1 = Pixel or 28--l.height / 2
				local Deformed1 = Deformed or 2
				local Deformed2, Pixel2 = Deformed1, Pixel1
				if type( Axis ) == "table" then
					Deformed2 = Axis[ 1 ]
					Pixel2 = Axis[ 2 ]
				end
				local Shape = ke4.shape.ASSDraw3( Shape )
				local shape_filter = function( x, y )
					local px, py = x, y
					if Axis == "x" then
						y = y - Pixel1 * sin( ((px - minx) / w_shape) * Deformed1 * pi )
					elseif Axis == "y" then
						x = x + Pixel1 * sin( ((py - miny) / h_shape) * Deformed1 * pi )
					else
						x = x + Pixel2 * sin( ((py - miny) / h_shape) * Deformed2 * pi )
						y = y - Pixel1 * sin( ((px - minx) / w_shape) * Deformed1 * pi )
					end
					return x, y
				end
				return format( "{\\p1}%s", ke4.shape.origin( ke4.shape.filter2( Shape, shape_filter, 2 ) ) )
			end, --{\p1}!_G.ke4.shape.deformed( _G.ke4.shape.rectangle, 8, 5, "y" )!
			
			deformed2 = function( Shape, Defor_x, Defor_y )
				--deforma los puntos internos de un conjunto de shapes matriz
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local Shape = Shape or shape.rectangle
				local coors = { }
				local deforx = Defor_x or 6 * ratio
				local defory = Defor_y or deforx
				local maxx, maxy = ke4.shape.width( Shape ), ke4.shape.height( Shape )
				Shape = ke4.shape.origin( Shape )
				Shape = Shape:gsub( "(%d+[%.%d]* %d+[%.%d]*)",
					function( Point )
						if table.inside( coors, Point ) == false then
							table.insert( coors, Point )
						end
						return "pnt" .. ke4.table.index( coors, Point )
					end
				)
				for i = 1, #coors do
					coors[ i ] = coors[ i ]:gsub( "(%d+[%.%d]*) (%d+[%.%d]*)",
						function( coor_x, coor_y )
							local coor_x = tonumber( coor_x )
							local coor_y = tonumber( coor_y )
							if coor_x ~= 0
								and coor_x ~= maxx then
								coor_x = coor_x + ke4.math.Rrs( deforx )
							end
							if coor_y ~= 0
								and coor_y ~= maxy then
								coor_y = coor_y + ke4.math.Rrs( defory )
							end
							if coor_x < 0 then
								coor_x = 0
							end
							if coor_x > maxx then
								coor_x = maxx
							end
							if coor_y < 0 then
								coor_y = 0
							end
							if coor_y > maxy then
								coor_y = maxy
							end
							return coor_x .. " " .. coor_y
						end
					)
				end
				local Shapes = { }
				for shp in Shape:gmatch( "m pnt%d+ [lb pnt%d]*" ) do
					shp = shp:gsub( "pnt(%d+)",
						function( Index )
							return coors[ tonumber( Index ) ]
						end
					)
					table.insert( Shapes, shp )
				end
				local Shapefx = ""
				for i = 1, #Shapes do
					Shapefx = Shapefx .. Shapes[ i ]
				end
				return Shapefx
			end,

			fusion = function( Shapes, Tags )
				-- reune las shapes individuales que conforman una shape, para retornarse en una única línea
				local shp_max_x, shp_max_y = { }, { }
				local Shapes_tbl = Shapes or { ke4.shape.rectangle, ke4.shape.circle }
				if type( Shapes_tbl ) == "string" then
					Shapes_tbl = ke4.shape.divide( ke4.shape.origin( Shapes_tbl ) )
				end
				for i = 1, #Shapes_tbl do
					ke4.shape.info( Shapes_tbl[ i ] )
					table.insert( shp_max_x, maxx )
					table.insert( shp_max_y, maxy )
				end
				local shp_mark_x = ke4.table.op( shp_max_x, "max" )
				local shp_mark_y = ke4.table.op( shp_max_y, "max" )
				local shp_mark = format( "m 0 0 l %s 0 m 0 %s l %s %s ", shp_mark_x, shp_mark_y, shp_mark_x, shp_mark_y )
				for i = 1, #Shapes_tbl do
					Shapes_tbl[ i ] = shp_mark .. Shapes_tbl[ i ]
				end
				for i = #Shapes_tbl, 1, -1 do
					Shapes_tbl[ i ] = ke4.shape.centerpos( Shapes_tbl[ i ], 0.5 * shp_mark_x + (#Shapes_tbl - i) * shp_mark_x, 0.5 * shp_mark_y )
					Shapes_tbl[ i ] = ke4.shape.displace( Shapes_tbl[ i ], -0.5 * #Shapes_tbl * shp_mark_x + 0.5 * shp_mark_x )
				end
				local shp_tags = Tags
				if Tags == nil then
					shp_tags = { }
					for i = 1, #Shapes_tbl do
						shp_tags[ i ] = format( "{\\1c%s}", ke4.random.color( nil, 82 ) )
					end
				end
				for i = 1, #Shapes_tbl do
					Shapes_tbl[ i ] = "{" .. shp_tags[ (i - 1) % #shp_tags + 1 ] .. "}" .. Shapes_tbl[ i ]
				end
				return ke4.table.op( Shapes_tbl, "concat" )
				-- colors = _G.ke4.table.ipol( { "&H0000FF&", "&H00FFFF&", "&H00FF00&", "&HFF00FF&", "&H0000FF&" },  24, "\\1c" )
				-- {\p1}!_G.ke4.shape.fusion( _G.ke4.shape.array( _G.ke4.shape.size( _G.ke4.shape.circle, 20, 6 ), 24, "radial", 32 ), colors )!
			end,
			
			filtershape = function( Shape, ... )
				--le aplica uno o más filtros (funciones) a cada una de
				--las shapes individuales que conforman a una Shape :D-
				local Shape = ke4.shape.ASSDraw3( Shape )
				local filters = { ... }
				if type( ... ) == "table" then
					filters = ...
				end
				if #filters == 0 then
					filters[ 1 ] = function( Shape_ii )
						return Shape_ii
					end
				end
				for i = 1, #filters do
					if type( filters[ i ] ) ~= "function" then
						filters[ i ] = function( Shape_ii )
							return Shape_ii
						end
					end
					local si = 0 --> contador de shapes individuales
					Shape = Shape:gsub( "m %-?%d+[%.%d]* %-?%d+[%.%-%dbl ]*",
						function( shp )
							si = si + 1
							return filters[ i ]( shp, si )
						end
					)
				end
				return Shape
				--Shp = _G.ke4.shape.array( _G.ke4.shape.size( _G.ke4.shape.rectangle, 10, 60 ), 20 );
				--Filterx = function( shp, si ) if si % 2 == 1 then return _G.ke4.shape.displace( shp, 0, -10 ) end return _G.ke4.shape.displace( shp, 0, 10 ) end
				--{\p1}!_G.ke4.shape.filtershape( Shp, Filterx )!
			end,
			
			from_audio = function( Audio, Time, Start, Width, Height, Thickness )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local Wav_fil = Audio or "test.wav"
				local Wav_frm = 2 * frame_dur
				local Wav_wth = Width or 120 * ratio
				local Wav_hht = (Height == nil) and 1 / 220 or 1 / Height --0.0045
				local Wav_tkn = Thickness or 4 * ratio
				local Wav_otm = Time or 1000
				if Wav_hht <= 0 then
					Wav_hht = 1 / 150
				end
				local function audio_to_shape( Samples, Width, Height_scale, Thickness )
					local thick, n = Thickness / 2, #Samples
					local Shape = format( "m 0 %s l", -thick )
					for i = 2, n do
						if i < n then
							Shape = format( "%s %s %s", Shape, (i - 1) / (n - 1) * Width, Samples[ i ] * Height_scale - thick )
						else
							Shape = format( "%s %s %s", Shape, (i - 1) / (n - 1) * Width, -thick )
						end
					end
					for i = n, 1, -1 do
						if i > 1
							and i < n then
							Shape = format( "%s %s %s", Shape, (i - 1) / (n - 1) * Width, Samples[ i ] * Height_scale + thick )
						else
							Shape = format( "%s %s %s", Shape, (i - 1) / (n - 1) * Width, thick )
						end
					end
					return Shape
				end
				if Wav_fil:match( "%.wav" ) == nil then
					Wav_fil = Wav_fil .. ".wav"
				end
				local reader = ke4.decode.create_wav_reader( Wav_fil )
				local chunk_size = reader.sample_from_ms( Wav_frm / 32 )
				local max_n, ms = ceil( Wav_otm / Wav_frm )
				local configs = { }
				for i = 1, max_n do
					ms = Start + Wav_frm * (i - 1)
					reader.position( floor( reader.sample_from_ms( ms ) ) )
					configs[ i ] = {
						ini = Wav_frm * (i - 1),
						fin = Wav_frm * i,
						shape = ke4.shape.ratio( audio_to_shape( reader.samples( chunk_size )[ 1 ], Wav_wth / 4, Wav_hht, Wav_tkn ), 4, 1 )
					}
				end
				return configs
				--code line: Audio = _G.ke4.shape.from_audio( "test.wav", line.duration, line.start_time , 400, 220 )
				--!maxloop( #Audio )!!retime( "preline", Audio[ j ].ini, Audio[ j ].fin )!{\an5\pos($x,$y)\bord0\shad0\p1}!Audio[ j ].shape!
			end,
			
			bounding = function( shape )
				-- Calculates shape bounding box
				if type( shape ) ~= "string" then
					error( "shape expected", 2 )
				end
				local x0, y0, x1, y1
				ke4.shape.filtery( shape, function( x, y )
					if x0 then
						x0, y0, x1, y1 = math.min( x0, x ), math.min( y0, y ), math.max( x1, x ), math.max( y1, y )
					else
						x0, y0, x1, y1 = x, y, x, y
					end
				end )
				return x0, y0, x1, y1
			end,
			
			detect = function( width, height, data, compare_func )
				-- Extracts shapes by similar data in 2d data map
				if type( width ) ~= "number" or floor( width ) ~= width or width < 1 or type( height ) ~= "number" or floor( height ) ~= height or height < 1 or type( data ) ~= "table" or #data < width * height or (compare_func ~= nil and type( compare_func ) ~= "function") then
					error( "width, height, data and optional data compare function expected", 2 )
				end
				if not compare_func then
					compare_func = function( a, b ) return a == b end
				end
				local data_n = width * height
				local elements = { n = 1, { value = data[ 1 ] } }
				for i = 2, data_n do
					for j = 1, elements.n do
						if compare_func( data[ i ], elements[ j ].value ) then
							goto trace_element_found
						end
					end
					elements.n = elements.n + 1
					elements[ elements.n ] = { value = type( data[ i ] ) == "table" and ke4.table.copy( data[ i ] ) or data[ i ] }
					::trace_element_found::
				end
				local function index_to_x( i )
					return (i - 1) % width
				end
				local function index_to_y( i )
					return floor( (i - 1) / width )
				end
				local function coord_to_index( x, y )
					return 1 + x + y * width
				end
				local function find_direction( bitmap, x, y, last_direction )
					local top_left, top_right, bottom_left, bottom_right =
						x-1 >= 0 and y - 1 >= 0 and bitmap[ coord_to_index( x - 1, y - 1 ) ] == 1 or false,
						x < width and y - 1 >= 0 and bitmap[ coord_to_index( x, y - 1 ) ] == 1 or false,
						x-1 >= 0 and y < height and bitmap[ coord_to_index( x - 1, y ) ] == 1 or false,
						x < width and y < height and bitmap[ coord_to_index( x, y ) ] == 1 or false
					return last_direction == 8 and (
							bottom_left and (
								top_left and top_right and 6 or
								top_left and 8 or
								4
							) or (	-- bottom_right
								top_left and top_right and 4 or
								top_right and 8 or
								6
							)
						) or last_direction == 6 and (
							top_left and (
								top_right and bottom_right and 2 or
								top_right and 6 or
								8
							)or (	-- bottom_left
								top_right and bottom_right and 8 or
								bottom_right and 6 or
								2
							)
						) or last_direction == 2 and (
							top_left and (
								bottom_left and bottom_right and 6 or
								bottom_left and 2 or
								4
							) or (	-- top_right
								bottom_left and bottom_right and 4 or
								bottom_right and 2 or
								6
							)
						) or last_direction == 4 and (
							top_right and (
								top_left and bottom_left and 2 or
								top_left and 4 or
								8
							) or (	-- bottom_right
								top_left and bottom_left and 8 or
								bottom_left and 4 or
								2
							)
						)
				end
				local function extract_contour( bitmap, x, y, cw )
					local contour, direction = { n = 1, cw and { x1 = x, y1 = y + 1, x2 = x, y2 = y, direction = 8 } or {x1 = x, y1 = y, x2 = x, y2 = y + 1, direction = 2 } }
					repeat
						x, y = contour[ contour.n ].x2, contour[ contour.n ].y2
						direction = find_direction( bitmap, x, y, contour[ contour.n ].direction )
						contour.n = contour.n + 1
						contour[ contour.n ] = { x1 = x, y1 = y, x2 = direction == 4 and x - 1 or direction == 6 and x + 1 or x, y2 = direction == 8 and y - 1 or direction == 2 and y + 1 or y, direction = direction }
					until contour[ contour.n ].x2 == contour[ 1 ].x1 and contour[ contour.n ].y2 == contour[ 1 ].y1
					return contour
				end
				local function contour_indices( contour )
					-- Get top & bottom line of contour
					local min_y, max_y, line
					for i=1, contour.n do
						line = contour[ i ]
						if line.direction == 8 then
							min_y, max_y = min_y and math.min( min_y, line.y2 ) or line.y2, max_y and math.max( max_y, line.y2 ) or line.y2
						elseif line.direction == 2 then
							min_y, max_y = min_y and math.min( min_y, line.y1 ) or line.y1, max_y and math.max( max_y, line.y1 ) or line.y1
						end
					end
					local indices, h_stops, h_stops_n, j = { n = 0 }
					for y = min_y, max_y do
						h_stops, h_stops_n = { }, 0
						for i = 1, contour.n do
							line = contour[ i ]
							if line.direction == 8 and line.y2 == y or line.direction == 2 and line.y1 == y then
								h_stops_n = h_stops_n + 1
								h_stops[ h_stops_n ] = line.x1
							end
						end
						table.sort( h_stops )
						for i = 1, h_stops_n, 2 do
							j = coord_to_index( h_stops[ i ], y )
							for x_off = 0, h_stops[ i + 1 ] - h_stops[ i ] - 1 do
								indices.n = indices.n + 1
								indices[ indices.n ] = j + x_off
							end
						end
					end
					return indices
				end
				local function merge_contour_lines( contour )
					local i = 1
					while i < contour.n do
						if contour[ i ].direction == contour[ i + 1 ].direction then
							contour[ i ].x2, contour[ i ].y2 = contour[ i + 1 ].x2, contour[ i + 1 ].y2
							table.remove( contour, i + 1 )
							contour.n = contour.n - 1
						else
							i = i + 1
						end
					end
					if contour.n > 1 and contour[ 1 ].direction == contour[ contour.n ].direction then
						contour[ 1 ].x1, contour[ 1 ].y1 = contour[ contour.n ].x1, contour[ contour.n ].y1
						table.remove( contour )
						contour.n = contour.n - 1
					end
					return contour
				end
				local function contour_to_shape( contour )
					local shape, shape_n, line = { format( "m %d %d l", contour[ 1 ].x1, contour[ 1 ].y1 ) }, 1
					for i = 1, contour.n do
						line = contour[ i ]
						shape_n = shape_n + 1
						shape[ shape_n ] = format( "%d %d", line.x2, line.y2 )
					end
					return table.concat( shape, " " )
				end
				local element, element_shapes, shape, shape_n, element_contour, element_hole_contour, indices, hole_indices
				local bitmap = { }
				for i = 1, elements.n do
					element, element_shapes = elements[ i ].value, { n = 0 }
					for i = 1, data_n do
						bitmap[ i ] = compare_func( data[ i ], element ) and 1 or 0
					end
					for i = 1, data_n do
						if bitmap[ i ] == 1 then
							element_contour = extract_contour( bitmap, index_to_x( i ), index_to_y( i ), true )
							indices = contour_indices( element_contour )
							shape, shape_n = { contour_to_shape( merge_contour_lines( element_contour ) ) }, 1
							for i = 1, indices.n do
								i = indices[ i ]
								if bitmap[ i ] == 0 then
									element_hole_contour = extract_contour( bitmap, index_to_x( i ), index_to_y( i ), false )
									hole_indices = contour_indices( element_hole_contour )
									shape_n = shape_n + 1
									shape[ shape_n ] = contour_to_shape( merge_contour_lines( element_hole_contour ) )
									for i = 1, hole_indices.n do
										i = hole_indices[ i ]
										bitmap[ i ] = bitmap[ i ] + 1
									end
								end
							end
							for i = 1, indices.n do
								i = indices[ i ]
								bitmap[ i ] = bitmap[ i ] - 1
							end
							element_shapes.n = element_shapes.n + 1
							element_shapes[ element_shapes.n ] = table.concat( shape, " " )
						end
					end
					elements[ i ].shapes = element_shapes
				end
				return elements
			end,
			
			filtery = function( shape, filter )
				-- Filters shape points
				if type( shape ) ~= "string" or type( filter ) ~= "function" then
					error( "shape and filter function expected", 2 )
				end
				local token_start, token_end, token, token_num = 1
				local point_start, typ, x, new_point
				repeat
					token_start, token_end, token = shape:find( "([^%s]+)", token_start )
					if token_start then
						token_num = tonumber( token )
						if not token_num then
							point_start, typ, x = token_start, token
						else
							if not x then
								if not point_start then
									point_start = token_start
								end
								x = token_num
							else
								x, token_num = filter( x, token_num, typ )
								if type( x ) == "number" and type( token_num ) == "number" then
									new_point = typ and format( "%s %s %s", typ, ke4.math.round( x, FP_PRECISION ), ke4.math.round( token_num, FP_PRECISION ) ) or
									format( "%s %s", ke4.math.round( x, FP_PRECISION ), ke4.math.round( token_num, FP_PRECISION ) )
									shape = format( "%s%s%s", shape:sub( 1, point_start - 1 ), new_point, shape:sub( token_end + 1 ) )
									token_end = point_start + #new_point - 1
								end
								point_start, typ, x = nil
							end
						end
						token_start = token_end + 1
					end
				until not token_start
				return shape
			end,
			
			flatten = function( shape, curve_tolerance )
				-- Converts shape curves to lines
				if type( shape ) ~= "string" then
					error( "shape expected", 2 )
				end
				local curve_tolerance = curve_tolerance or CURVE_TOLERANCE
				local function curve4_subdivide( x0, y0, x1, y1, x2, y2, x3, y3, pct )
					local x01, y01, x12, y12, x23, y23 = (x0 + x1) * pct, (y0 + y1) * pct, (x1 + x2) * pct, (y1 + y2) * pct, (x2 + x3) * pct, (y2 + y3) * pct
					local x012, y012, x123, y123 = (x01 + x12) * pct, (y01 + y12) * pct, (x12 + x23) * pct, (y12 + y23) * pct
					local x0123, y0123 = (x012 + x123) * pct, (y012 + y123) * pct
					return x0, y0, x01, y01, x012, y012, x0123, y0123, x0123, y0123, x123, y123, x23, y23, x3, y3
				end
				local function curve4_is_flat( x0, y0, x1, y1, x2, y2, x3, y3, tolerance )
					local vecs = { { x1 - x0, y1 - y0 }, { x2 - x1, y2 - y1 }, { x3 - x2, y3 - y2 } }
					local i, n = 1, #vecs
					while i <= n do
						if vecs[ i ][ 1 ] == 0 and vecs[ i ][ 2 ] == 0 then
							table.remove( vecs, i )
							n = n - 1
						else
							i = i + 1
						end
					end
					for i = 2, n do
						if abs( ke4.math.degree( vecs[ i - 1 ][ 1 ], vecs[ i - 1 ][ 2 ], 0, vecs[ i ][ 1 ], vecs[ i ][ 2 ], 0 ) ) > tolerance then
							return false
						end
					end
					return true
				end
				local function curve4_to_lines( x0, y0, x1, y1, x2, y2, x3, y3 )
					local pts, pts_n = { x0, y0 }, 2
					local function convert_recursive( x0, y0, x1, y1, x2, y2, x3, y3 )
						if curve4_is_flat( x0, y0, x1, y1, x2, y2, x3, y3, curve_tolerance ) then
							pts[ pts_n + 1 ] = x3
							pts[ pts_n + 2 ] = y3
							pts_n = pts_n + 2
						else
							local x10, y10, x11, y11, x12, y12, x13, y13, x20, y20, x21, y21, x22, y22, x23, y23 = curve4_subdivide( x0, y0, x1, y1, x2, y2, x3, y3, 0.5 )
							convert_recursive( x10, y10, x11, y11, x12, y12, x13, y13 )
							convert_recursive( x20, y20, x21, y21, x22, y22, x23, y23 )
						end
					end
					convert_recursive( x0, y0, x1, y1, x2, y2, x3, y3 )
					return pts
				end
				local curves_start, curves_end, x0, y0 = 1
				local curve_start, curve_end, x1, y1, x2, y2, x3, y3
				local line_points, line_curve
				repeat
					curves_start, curves_end, x0, y0 = shape:find( "([^%s]+)%s+([^%s]+)%s+b%s+", curves_start )
					x0, y0 = tonumber( x0 ), tonumber( y0 )
					if x0 and y0 then
						shape = shape:sub( 1, curves_start - 1 ) .. shape:sub( curves_start ):gsub( "b", "l", 1 )
						curve_start = curves_end + 1
						repeat
							curve_start, curve_end, x1, y1, x2, y2, x3, y3 = shape:find( "([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+([^%s]+)", curve_start )
							x1, y1, x2, y2, x3, y3 = tonumber( x1 ), tonumber( y1 ), tonumber( x2 ), tonumber( y2 ), tonumber( x3 ), tonumber( y3 )
							if x1 and y1 and x2 and y2 and x3 and y3 then
								local line_points = curve4_to_lines( x0, y0, x1, y1, x2, y2, x3, y3 )
								for i = 1, #line_points do
									line_points[ i ] = ke4.math.round( line_points[ i ], FP_PRECISION )
								end
								line_curve = table.concat(line_points, " ")
								shape = format( "%s%s%s", shape:sub( 1, curve_start - 1 ), line_curve, shape:sub( curve_end + 1 ) )
								curve_end = curve_start + #line_curve - 1
								x0, y0 = x3, y3
								curve_start = curve_end + 1
							end
						until not (x1 and y1 and x2 and y2 and x3 and y3)
						curves_start = curves_end + 1
					end
				until not (x0 and y0)
				return shape
			end,
			
			glue = function( src_shape, dst_shape, transform_callback )
				-- Projects shape on shape
				if type( src_shape ) ~= "string" or type( dst_shape ) ~= "string" or (transform_callback ~= nil and type( transform_callback ) ~= "function") then
					error( "2 shapes and optional callback function expected", 2 )
				end
				local _, figure_end = dst_shape:find( "^%s*m.-m" )
				if figure_end then
					dst_shape = dst_shape:sub( 1, figure_end - 1 )
				end
				local dst_lines, dst_lines_n = { }, 0
				local dst_lines_length, dst_line, last_point = 0
				ke4.shape.filtery( ke4.shape.flatten( dst_shape ), function( x, y )
					if last_point then
						dst_line = { last_point[ 1 ], last_point[ 2 ], x - last_point[ 1 ], y - last_point[ 2 ], ke4.math.distance2( x - last_point[ 1 ], y - last_point[ 2 ] ) }
						if dst_line[ 5 ] > 0 then
							dst_lines_n = dst_lines_n + 1
							dst_lines[ dst_lines_n ] = dst_line
							dst_lines_length = dst_lines_length + dst_line[ 5 ]
						end
					end
					last_point = { x, y }
				end )
				if dst_lines_n > 0 then
					local cur_length = 0
					for _, dst_line in ipairs( dst_lines ) do
						dst_line[ 6 ] = cur_length / dst_lines_length
						cur_length = cur_length + dst_line[ 5 ]
						dst_line[ 7 ] = cur_length / dst_lines_length
					end
					local x0, _, x1, y1 = ke4.shape.bounding( ke4.shape.flatten( src_shape ) )
					if x0 and x1 > x0 then
						local w = x1 - x0
						local x_pct, y_off, x_pct_temp, y_off_temp
						local dst_line_pos, ovec_x, ovec_y
						return ke4.shape.filtery( src_shape, function( x, y )
							x_pct, y_off = (x - x0) / w, y - y1
							if transform_callback then
								x_pct_temp, y_off_temp = transform_callback( x_pct, y_off )
								if type( x_pct_temp ) == "number" and type( y_off_temp ) == "number" then
									x_pct, y_off = math.max( 0, math.min( x_pct_temp, 1 ) ), y_off_temp
								end
							end
							for i = 1, dst_lines_n do
								dst_line = dst_lines[ i ]
								if x_pct >= dst_line[ 6 ] and x_pct <= dst_line[ 7 ] then
									dst_line_pos = (x_pct - dst_line[ 6 ]) / (dst_line[ 7 ] - dst_line[ 6 ])
									ovec_x, ovec_y = ke4.math.ortho( dst_line[ 3 ], dst_line[ 4 ], 0, 0, 0, -1 )
									ovec_x, ovec_y = ke4.math.stretch( ovec_x, ovec_y, 0, y_off )
									return dst_line[ 1 ] + dst_line_pos * dst_line[ 3 ] + ovec_x,
											dst_line[ 2 ] + dst_line_pos * dst_line[ 4 ] + ovec_y
								end
							end
						end )
					end
				end
			end,
			
			move = function( shape, x, y )
				-- Shifts shape coordinates
				if type( shape ) ~= "string" or type( x ) ~= "number" or type( y ) ~= "number" then
					error( "shape, horizontal shift and vertical shift expected", 2 )
				end
				return ke4.shape.filtery( shape, function( cx, cy )
					return cx + x, cy + y
				end )
			end,
			
			split = function( shape, max_len )
				-- Splits shape lines into shorter segments
				if type( shape ) ~= "string" or type( max_len ) ~= "number" or max_len <= 0 then
					error( "shape and maximal line length expected", 2 )
				end
				shape = shape:gsub( "%s+c", "" )
				local function line_split( x0, y0, x1, y1 )
					local rel_x, rel_y = x1 - x0, y1 - y0
					local distance = ke4.math.distance2( rel_x, rel_y )
					if distance > max_len then
						local lines, lines_n, distance_rest, pct = { }, 0, distance % max_len
						for cur_distance = distance_rest > 0 and distance_rest or max_len, distance, max_len do
							pct = cur_distance / distance
							lines_n = lines_n + 1
							lines[ lines_n ] = format("%s %s", ke4.math.round( x0 + rel_x * pct, FP_PRECISION ), ke4.math.round( y0 + rel_y * pct, FP_PRECISION ) )
						end
						return table.concat( lines, " " )
					else
						return format( "%s %s", ke4.math.round( x1, FP_PRECISION ), ke4.math.round( y1, FP_PRECISION ) )
					end
				end
				local new_shape, new_shape_n = { }, 0
				local line_mode, last_point, last_move
				ke4.shape.filtery( shape, function( x, y, typ )
					if typ == "m" and last_move and not (last_point[ 1 ] == last_move[ 1 ] and last_point[ 2 ] == last_move[ 2 ]) then
						if not line_mode then
							new_shape_n = new_shape_n + 1
							new_shape[ new_shape_n ] =  "l"
						end
						new_shape_n = new_shape_n + 1
						new_shape[ new_shape_n ] = line_split( last_point[ 1 ], last_point[ 2 ], last_move[ 1 ], last_move[ 2 ] )
					end
					if typ then
						new_shape_n = new_shape_n + 1
						new_shape[ new_shape_n ] = typ
					end
					if typ then
						line_mode = typ == "l"
					end
					new_shape_n = new_shape_n + 1
					new_shape[ new_shape_n ] = line_mode and last_point and line_split( last_point[ 1 ], last_point[ 2 ], x, y ) or format( "%s %s", ke4.math.round( x, FP_PRECISION ), ke4.math.round( y, FP_PRECISION ) )
					last_point = { x, y }
					if typ == "m" then
						last_move = { x, y }
					end
				end )
				if last_move and not (last_point[ 1 ] == last_move[ 1 ] and last_point[ 2 ] == last_move[ 2 ]) then
					if not line_mode then
						new_shape_n = new_shape_n + 1
						new_shape[ new_shape_n ] =  "l"
					end
					new_shape_n = new_shape_n + 1
					new_shape[ new_shape_n ] = line_split( last_point[ 1 ], last_point[ 2 ], last_move[ 1 ], last_move[ 2 ] )
				end
				return table.concat( new_shape, " " )
			end,
			
			to_outline2 = function( shape, width_xy, width_y, mode )
				-- Converts shape to stroke version
				if type( shape ) ~= "string" or type( width_xy ) ~= "number" or width_y ~= nil and type( width_y ) ~= "number" or mode ~= nil and type( mode ) ~= "string" then
					error( "shape, line width (general or horizontal and vertical) and optional mode expected", 2 )
				elseif width_y and (width_xy < 0 or width_y < 0 or not (width_xy > 0 or width_y > 0)) or width_xy <= 0 then
					error( "one width must be >0", 2 )
				elseif mode and mode ~= "round" and mode ~= "bevel" and mode ~= "miter" then
					error( "valid mode expected", 2 )
				end
				local width, xscale, yscale
				if width_y and width_xy ~= width_y then
					width = math.max( width_xy, width_y )
					xscale, yscale = width_xy / width, width_y / width
				else
					width, xscale, yscale = width_xy, 1, 1
				end
				local figures, figures_n, figure, figure_n = { }, 0, { }, 0
				local last_move
				ke4.shape.filtery( shape, function( x, y, typ )
					if typ and not ( typ == "m" or typ == "l" ) then
						error( "shape have to contain only \"moves\" and \"lines\"", 2 )
					end
					if not last_move or typ == "m" then
						if figure_n > 2 then
							if last_move and figure[ figure_n ][ 1 ] == last_move[ 1 ] and figure[ figure_n ][ 2 ] == last_move[ 2 ] then
								figure[ figure_n ] = nil
							end
							figures_n = figures_n + 1
							figures[ figures_n ] = figure
						end
						figure, figure_n = { }, 0
						last_move = { x, y }
					end
					if figure_n == 0 or not(figure[ figure_n ][ 1 ] == x and figure[ figure_n ][ 2 ] == y) then
						figure_n = figure_n + 1
						figure[ figure_n ] = { x, y }
					end
				end )
				if figure_n > 2 then
					if last_move and figure[ figure_n ][ 1 ] == last_move[ 1 ] and figure[ figure_n ][ 2 ] == last_move[ 2 ] then
						figure[ figure_n ] = nil
					end
					figures_n = figures_n + 1
					figures[ figures_n ] = figure
				end
				local stroke_shape, stroke_shape_n = { }, 0
				for fi, figure in ipairs( figures ) do
					for i = 1, 2 do
						local outline, outline_n = { }, 0
						local loop_begin, loop_end, loop_steps
						if i == 1 then
							loop_begin, loop_end, loop_step = #figure, 1, -1
						else
							loop_begin, loop_end, loop_step = 1, #figure, 1
						end
						for k = loop_begin, loop_end, loop_step do
							local point = figure[ k ]
							local pre_point, post_point
							if i == 1 then
								if k == 1 then
									pre_point = figure[ k + 1 ]
									post_point = figure[ #figure ]
								elseif k == #figure then
									pre_point = figure[ 1 ]
									post_point = figure[ k - 1 ]
								else
									pre_point = figure[ k + 1 ]
									post_point = figure[ k - 1 ]
								end
							else
								if k == 1 then
									pre_point = figure[ #figure ]
									post_point = figure[ k + 1 ]
								elseif k == #figure then
									pre_point = figure[ k - 1 ]
									post_point = figure[ 1 ]
								else
									pre_point = figure[ k - 1 ]
									post_point = figure[ k + 1 ]
								end
							end
							local vec1_x, vec1_y = point[ 1 ] - pre_point[ 1 ], point[ 2 ] - pre_point[ 2 ]
							local vec2_x, vec2_y = point[ 1 ] - post_point[ 1 ], point[ 2 ] - post_point[ 2 ]
							local o_vec1_x, o_vec1_y = ke4.math.ortho( vec1_x, vec1_y, 0, 0, 0, 1 )
							o_vec1_x, o_vec1_y = ke4.math.stretch( o_vec1_x, o_vec1_y, 0, width )
							local o_vec2_x, o_vec2_y = ke4.math.ortho( vec2_x, vec2_y, 0, 0, 0, -1 )
							o_vec2_x, o_vec2_y = ke4.math.stretch( o_vec2_x, o_vec2_y, 0, width )
							local is_x, is_y = ke4.math.line_intersect( point[ 1 ] + o_vec1_x - vec1_x, point[ 2 ] + o_vec1_y - vec1_y,
																						point[ 1 ] + o_vec1_x, point[ 2 ] + o_vec1_y,
																						point[ 1 ] + o_vec2_x - vec2_x, point[ 2 ] + o_vec2_y - vec2_y,
																						point[ 1 ] + o_vec2_x, point[ 2 ] + o_vec2_y,
																						true )
							if is_y then
								outline_n = outline_n + 1
								outline[ outline_n ] = format( "%s%s %s",
																			outline_n == 1 and "m " or outline_n == 2 and "l " or "",
																			ke4.math.round( point[ 1 ] + (is_x - point[ 1 ]) * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + (is_y - point[ 2 ]) * yscale, FP_PRECISION ) )
							else
								outline_n = outline_n + 1
								outline[ outline_n ] = format( "%s%s %s",
																			outline_n == 1 and "m " or outline_n == 2 and "l " or "",
																			ke4.math.round( point[ 1 ] + o_vec1_x * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + o_vec1_y * yscale, FP_PRECISION ) )
								if mode == "bevel" then
									-- Nothing to add!
								elseif mode == "miter" then
									is_x, is_y = ke4.math.line_intersect( point[ 1 ] + o_vec1_x - vec1_x, point[ 2 ] + o_vec1_y - vec1_y,
																						point[ 1 ] + o_vec1_x, point[ 2 ] + o_vec1_y,
																						point[ 1 ] + o_vec2_x - vec2_x, point[ 2 ] + o_vec2_y - vec2_y,
																						point[ 1 ] + o_vec2_x, point[ 2 ] + o_vec2_y )
									if is_y then
										local is_vec_x, is_vec_y = is_x - point[ 1 ], is_y - point[ 2 ]
										local is_vec_len = ke4.math.distance2( is_vec_x, is_vec_y )
										if is_vec_len > MITER_LIMIT then
											local fix_scale = MITER_LIMIT / is_vec_len
											outline_n = outline_n + 1
											outline[ outline_n ] = format( "%s%s %s %s %s",
																						outline_n == 2 and "l " or "",
																						ke4.math.round( point[ 1 ] + (o_vec1_x + (is_vec_x - o_vec1_x) * fix_scale) * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + (o_vec1_y + (is_vec_y - o_vec1_y) * fix_scale) * yscale, FP_PRECISION ),
																						ke4.math.round( point[ 1 ] + (o_vec2_x + (is_vec_x - o_vec2_x) * fix_scale) * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + (o_vec2_y + (is_vec_y - o_vec2_y) * fix_scale) * yscale, FP_PRECISION ) )
										else
											outline_n = outline_n + 1
											outline[ outline_n ] = format( "%s%s %s",
																						outline_n == 2 and "l " or "",
																						ke4.math.round( point[ 1 ] + is_vec_x * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + is_vec_y * yscale, FP_PRECISION ) )
										end
									else	-- Parallel vectors
										vec1_x, vec1_y = ke4.math.stretch( vec1_x, vec1_y, 0, MITER_LIMIT )
										vec2_x, vec2_y = ke4.math.stretch( vec2_x, vec2_y, 0, MITER_LIMIT )
										outline_n = outline_n + 1
										outline[ outline_n ] = format("%s%s %s %s %s",
																					outline_n == 2 and "l " or "",
																					ke4.math.round( point[ 1 ] + (o_vec1_x + vec1_x) * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + (o_vec1_y + vec1_y) * yscale, FP_PRECISION ),
																					ke4.math.round( point[ 1 ] + (o_vec2_x + vec2_x) * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + (o_vec2_y + vec2_y) * yscale, FP_PRECISION ) )
									end
								else
									local degree = ke4.math.degree( o_vec1_x, o_vec1_y, 0, o_vec2_x, o_vec2_y, 0 )
									local circ = abs( rad( degree ) ) * width
									if circ > MAX_CIRCUMFERENCE then
										local circ_rest = circ % MAX_CIRCUMFERENCE
										for cur_circ = circ_rest > 0 and circ_rest or MAX_CIRCUMFERENCE, circ - MAX_CIRCUMFERENCE, MAX_CIRCUMFERENCE do
											local curve_vec_x, curve_vec_y = rotate2d( o_vec1_x, o_vec1_y, cur_circ / circ * degree )
											outline_n = outline_n + 1
											outline[ outline_n ] = format( "%s%s %s",
																						outline_n == 2 and "l " or "",
																						ke4.math.round( point[ 1 ] + curve_vec_x * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + curve_vec_y * yscale, FP_PRECISION ) )
										end
									end
								end
								outline_n = outline_n + 1
								outline[ outline_n ] = format( "%s%s %s",
																			outline_n == 2 and "l " or "",
																			ke4.math.round( point[ 1 ] + o_vec2_x * xscale, FP_PRECISION ), ke4.math.round( point[ 2 ] + o_vec2_y * yscale, FP_PRECISION ) )
							end
						end
						stroke_shape_n = stroke_shape_n + 1
						stroke_shape[ stroke_shape_n ] = table.concat( outline, " " )
					end
				end
				return table.concat( stroke_shape, " " )
			end,
			
			to_pixels2 = function( shape )
				-- Converts shape to pixels
				if type( shape ) ~= "string" then
					error( "shape expected", 2 )
				end
				local upscale = SUPERSAMPLING
				local downscale = 1 / upscale
				shape = ke4.shape.filtery( shape, function( x, y )
					return x * upscale, y * upscale
				end )
				local x1, y1, x2, y2 = ke4.shape.bounding( shape )
				if not y2 then
					error( "not enough shape points", 2 )
				end
				local shift_x, shift_y = -(x1 - x1 % upscale), -(y1 - y1 % upscale)
				shape = ke4.shape.move( shape, shift_x, shift_y )
				local function render_shape( width, height, image, shape )
					local lines, lines_n, last_point, last_move = { }, 0
					ke4.shape.filtery( ke4.shape.flatten( shape ), function( x, y, typ )
						x, y = ke4.math.round( x ), ke4.math.round( y )
						if typ == "m" then
							if last_move and last_move[ 2 ] ~= last_point[ 2 ] and not (last_point[ 2 ] < 0 and last_move[ 2 ] < 0) and not (last_point[ 2 ] > height and last_move[ 2 ] > height) then
								lines_n = lines_n + 1
								lines[ lines_n ] = { last_point[ 1 ], last_point[ 2 ], last_move[ 1 ] - last_point[ 1 ], last_move[ 2 ] - last_point[ 2 ] }
							end
							last_move = { x, y }
						elseif last_point and last_point[ 2 ] ~= y and not (last_point[ 2 ] < 0 and y < 0) and not (last_point[ 2 ] > height and y > height) then
							lines_n = lines_n + 1
							lines[ lines_n ] = { last_point[ 1 ], last_point[ 2 ], x - last_point[ 1 ], y - last_point[ 2 ] }
						end
						last_point = { x, y }
					end )
					if last_move and last_move[ 2 ] ~= last_point[ 2 ] and not (last_point[ 2 ] < 0 and last_move[ 2 ] < 0) and not (last_point[ 2 ] > height and last_move[ 2 ] > height) then
						lines_n = lines_n + 1
						lines[ lines_n ] = { last_point[ 1 ], last_point[ 2 ], last_move[ 1 ] - last_point[ 1 ], last_move[ 2 ] - last_point[ 2 ] }
					end
					local function line_x_hline( x, y, vx, vy, y2 )
						if vy ~= 0 then
							local s = (y2 - y) / vy
							if s >= 0 and s <= 1 then
								return x + s * vx, y2
							end
						end
					end
					local _, y1, _, y2 = ke4.shape.bounding( shape )
					for y = math.max( floor( y1 ), 0), math.min( ceil( y2 ), height ) - 1 do
						local row_stops, row_stops_n = { }, 0
						for i = 1, lines_n do
							local line = lines[ i ]
							local cx = line_x_hline( line[ 1 ], line[ 2 ], line[ 3 ], line[ 4 ], y + 0.5 )
							if cx then
								row_stops_n = row_stops_n + 1
								row_stops[ row_stops_n ] = { ke4.math.trim( cx, 0, width ), line[ 4 ] > 0 and 1 or -1 }
							end
						end
						if row_stops_n > 1 then
							table.sort( row_stops, function( a, b )
								return a[ 1 ] < b[ 1 ]
							end )
							local status, row_index = 0, 1 + y * width
							for i = 1, row_stops_n - 1 do
								status = status + row_stops[ i ][ 2 ]
								if status ~= 0 then
									for x = ceil( row_stops[ i ][ 1 ] - 0.5 ), floor( row_stops[ i + 1 ][ 1 ] + 0.5 ) - 1 do
										image[ row_index + x ] = true
									end
								end
							end
						end
					end
				end
				local img_width, img_height, img_data = ceil( (x2 + shift_x) * downscale ) * upscale, ceil( (y2 + shift_y) * downscale ) * upscale, { }
				for i = 1, img_width * img_height do
					img_data[ i ] = false
				end
				render_shape( img_width, img_height, img_data, shape )
				local pixels, pixels_n, opacity = { }, 0
				for y = 0, img_height - upscale, upscale do
					for x = 0, img_width - upscale, upscale do
						opacity = 0
						for yy = 0, upscale - 1 do
							for xx = 0, upscale - 1 do
								if img_data[ 1 + (y + yy) * img_width + (x + xx) ] then
									opacity = opacity + 255
								end
							end
						end
						if opacity > 0 then
							pixels_n = pixels_n + 1
							pixels[ pixels_n ] = {
								alpha = opacity * (downscale * downscale),
								x = (x - shift_x) * downscale,
								y = (y - shift_y) * downscale
							}
						end
					end
				end
				return pixels
			end,
			
			transform = function( shape, matrix )
				-- Applies matrix to shape coordinates
				if type( shape ) ~= "string" or type( matrix ) ~= "table" or type( matrix.transform ) ~= "function" then
					error( "shape and matrix expected", 2 )
				end
				local success, x, y, z, w = pcall( matrix.transform, 1, 1, 1 )
				if not success or type( x ) ~= "number" or type( y ) ~= "number" or type( z ) ~= "number" or type( w ) ~= "number" then
					error( "matrix transform method invalid", 2 )
				end
				return ke4.shape.filtery( shape, function( x, y )
					x, y, z, w = matrix.transform( x, y, 0 )
					return x / w, y / w
				end )
			end,
			
			to_clip = function( Shape, Pos_x, Pos_y, Ratio, Iclip )
				local Shape = ke4.shape.ASSDraw3( Shape )
				local Ratio = Ratio or 1
				if type( Ratio ) == "number" then
					Shape = ke4.shape.ratio( Shape, Ratio )
				elseif type( Ratio ) == "table" then
					Shape = ke4.shape.size( Shape, Ratio[ 1 ] )
				end
				Shape = ke4.shape.centerpos( Shape, Pos_x, Pos_y )
				if Iclip then
					return format( "\\iclip(%s)", Shape )
				end
				return format( "\\clip(%s)", Shape )
			end, --{\an5\pos($x,$y)!_G.ke4.shape.to_clip( _G.ke4.shape.circle, $x, $y )!}
			
			intersect = function( Shape1, Shape2 )
				--retorna una tabla con los puntos de intersección entre dos shapes
				local Shape1 = ke4.shape.ASSDraw3( ke4.shape.flatten( Shape1, 4 ) )
				local Shape2 = ke4.shape.ASSDraw3( ke4.shape.flatten( Shape2, 4 ) )
				--------------------------------
				local function line_shp( Shape )
					--obtiene los segmentos de recta que posee una shape
					local parts_shp, line = { }
					for line in Shape:gmatch( "[mbl]^*%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*" ) do
						parts_shp[ #parts_shp + 1 ] = line
					end
					local lines_tbl = { }
					for i = 1, #parts_shp do
						if parts_shp[ i ]:sub( 1, 1 ) == "l" then
							lines_tbl[ #lines_tbl + 1 ] = parts_shp[ i - 1 ]:match( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*" ) .. " " .. parts_shp[ i ]
						end
					end
					return lines_tbl
				end
				------------------------------
				local function minmax( Shape )
					--retorna los valores mínimos y máximos en "x" y "y" de una shape
					local minx, maxx = 1000000, -1000000
					local miny, maxy = 1000000, -1000000
					local Shape = Shape:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)",
						function( val_x, val_y )
							minx, miny = math.min( minx, val_x ), math.min( miny, val_y )
							maxx, maxy = math.max( maxx, val_x ), math.max( maxy, val_y )
						end
					)
					return minx, miny, maxx, maxy
				end
				-----------------------------------------
				local Lines_shape1 = line_shp( Shape1 )
				local Lines_shape2 = line_shp( Shape2 )
				-----------------------------------------
				local function intersectx( Line1, Line2 )
					--retorna el punto de intersección (si lo hay) entre dos segmentos rectos de shape :D
					local x1, y1, x2, y2 = Line1:match( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)%s+l%s+(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)" )
					local x3, y3, x4, y4 = Line2:match( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)%s+l%s+(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)" )
					x1, y1, x2, y2 = tonumber( x1 ), tonumber( y1 ), tonumber( x2 ), tonumber( y2 )
					x3, y3, x4, y4 = tonumber( x3 ), tonumber( y3 ), tonumber( x4 ), tonumber( y4 )
					local x, y = ke4.math.intersect( x1, y1, x2, y2, x3, y3, x4, y4 )
					local minx1, miny1, maxx1, maxy1 = minmax( Line1 )
					local minx2, miny2, maxx2, maxy2 = minmax( Line2 )
					local min_x, max_x = math.max( minx1, minx2 ), math.min( maxx1, maxx2 )
					local min_y, max_y = math.max( miny1, miny2 ), math.min( maxy1, maxy2 )
					if type( x ) == "number"
						and (x >= min_x and x <= max_x)
						and (y >= min_y and y <= max_y) then
						return { x, y }
					end
					return "not x and y intersect"
				end
				-----------------------------------------
				local intersect_tbl = { }
				for i = 1, #Lines_shape1 do
					for k = 1, #Lines_shape2 do
						if type( intersectx( Lines_shape1[ i ], Lines_shape2[ k ] ) ) == "table" then
							intersect_tbl[ #intersect_tbl + 1 ] = intersectx( Lines_shape1[ i ], Lines_shape2[ k ] )
						end
					end
				end
				return intersect_tbl
			end, --!_G.ke4.table.view( _G.ke4.shape.intersect( "m 0 0 l 0 20 l 20 20 l 20 0 l 0 0 ", "m 15 -6 l 4 24 l 36 5 l 15 -6 " ) )!
			
			insert = function( Shape1, Shape2 )
				--inserta mutuamente el código de una shape en la otra
				local Shape1 = Shape1 or ke4.shape.rectangle
				local Shape2 = Shape2 or ke4.shape.circle
				-------------------------------------
				local function parts( Shape )
					--segmentos de una shape
					local Parts = { }
					for p in Shape:gmatch( "[mlb]^*%s+%-?%d+[%.%d]*%s+[%-%.%d ]*" ) do
						table.insert( Parts, p )
					end
					return Parts
				end
				-------------------------------------
				local function valxy( Part_1, Part_2, Part_3 )
					local new_part1, new_part2 = ""
					local Part_3 = Part_3 or Part_2
					local type_part_1 = Part_1:match( "[mlb]^*" )	--tipo de segmento de shape 1
					local type_part_2 = Part_2:match( "[mlb]^*" )	--tipo de segmento de shape 2
					local type_part_3 = Part_3:match( "[mlb]^*" )	--tipo de segmento de shape 2
					if type_part_1 == type_part_2 then
						--si son del mismo tipo, retorna ambos segmentos
						return { Part_1, Part_2 }
					end
					local xpoint1, xpoint2 --punto de referencia del segmento 1
					if type_part_1 == "m"
						or type_part_1 == "l" then
						--toma en cuenta las dos coordenadas del segmento
						xpoint1 = Part_1:match( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*" )
					else --toma en cuenta las dos últimas coordenadas del segmento
						xpoint1 = Part_1:match( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+(%-?%d+[%.%d]*%s+%-?%d+[%.%d]*)" )
					end
					if type_part_3 == "m"
						or type_part_3 == "l" then
						--toma en cuenta las dos coordenadas del segmento
						xpoint2 = Part_3:match( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*" )
					else --toma en cuenta las dos últimas coordenadas del segmento
						xpoint2 = Part_3:match( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+%-?%d+[%.%d]*%s+(%-?%d+[%.%d]*%s+%-?%d+[%.%d]*)" )
					end
					--segmento 2 insertado en el 1
					if type_part_2 == "m"
						or type_part_2 == "l" then
						new_part1 = Part_1 .. type_part_2 .. " " .. xpoint1 .. " "
					else
						new_part1 = Part_1 .. type_part_2 .. " " .. xpoint1 .. " " .. xpoint1 .. " " .. xpoint1 .. " "
					end
					--segmento 1 insertado en el 2
					if type_part_1 == "m"
						or type_part_1 == "l" then
						new_part2 = type_part_1 .. " " .. xpoint2 .. " " .. Part_2
					else
						new_part2 = type_part_1 .. " " .. xpoint2 .. " " .. xpoint2 .. " " .. xpoint2 .. " " .. Part_2
					end
					return { new_part1, new_part2 }
				end
				-------------------------------------
				local Parts_1 = parts( Shape1 )				--segmentos de la shape 1
				local Parts_2 = parts( Shape2 )				--segmentos de la shape 2
				local Shape_fx1, Shape_fx2 = { }, { }		--modificaciones de las shapes 1 y 2
				local last_point_1 = Parts_1[ #Parts_1 ]	--último segmento de la shape 1
				local last_point_2 = Parts_2[ #Parts_2 ]	--último segmento de la shape 2
				local max_n = math.max( #Parts_1, #Parts_2 )
				for i = 2, max_n do
					if Parts_1[ i ]
						and Parts_2[ i ] then
						Shape_fx1[ i ] = valxy( Parts_1[ i ], Parts_2[ i ] )[ 1 ]
						Shape_fx2[ i ] = valxy( Parts_1[ i ], Parts_2[ i ], Parts_2[ i - 1 ] )[ 2 ]
					elseif Parts_1[ i ] then
						Shape_fx1[ i ] = valxy( Parts_1[ i ], last_point_2 )[ 1 ]
						Shape_fx2[ i ] = valxy( Parts_1[ i ], last_point_2 )[ 2 ]
					else
						Shape_fx1[ i ] = valxy( last_point_1, Parts_2[ i ] )[ 1 ]
						Shape_fx2[ i ] = valxy( last_point_1, Parts_2[ i ], Parts_2[ i - 1 ] )[ 2 ]
					end
				end
				Shape_fx1[ 1 ], Shape_fx2[ 1 ] = Parts_1[ 1 ], Parts_2[ 1 ]
				return table.concat( Shape_fx1 ), table.concat( Shape_fx2 ) --november 22nd 2019
			end, --ke4.shape.insert2( shape.rectangle, shape.circle )

		},
		
		-- Advanced substation alpha sublibrary
		ass = {
			convert_time = function( ass_ms )
				-- Converts between milliseconds and ASS timestamp
				if type( ass_ms ) == "number" and ass_ms >= 0 then	-- Milliseconds
					return format("%d:%02d:%02d.%02d",
												floor( ass_ms / 3600000 ) % 10,
												floor( ass_ms % 3600000 / 60000 ),
												floor( ass_ms % 60000 / 1000 ),
												floor( ass_ms % 1000 / 10 ) )
				elseif type( ass_ms ) == "string" and ass_ms:find("^%d:%d%d:%d%d%.%d%d$") then	-- ASS timestamp
					return ass_ms:sub( 1, 1 ) * 3600000 + ass_ms:sub( 3, 4 ) * 60000 + ass_ms:sub( 6, 7 ) * 1000 + ass_ms:sub( 9, 10 ) * 10
				else
					error( "milliseconds or ASS timestamp expected", 2 )
				end
			end,
			
			convert_coloralpha = function( ass_r_a, g, b, a )
				-- Converts between color &/+ alpha numeric and ASS color &/+ alpha
				if type( ass_r_a ) == "number" and ass_r_a >= 0 and ass_r_a <= 255 then	-- Alpha / red numeric
					if type( g ) == "number" and g >= 0 and g <= 255 and type( b ) == "number" and b >= 0 and b <= 255 then	-- Green + blue numeric
						if type( a ) == "number" and a >= 0 and a <= 255 then	-- Alpha numeric
							return format( "&H%02X%02X%02X%02X", 255 - a, b, g, ass_r_a )
						else
							return format( "&H%02X%02X%02X&", b, g, ass_r_a )
						end
					else
						return format( "&H%02X&", 255 - ass_r_a )
					end
				elseif type( ass_r_a ) == "string" then	-- ASS value
					if ass_r_a:find( "^&H%x%x&$" ) then	-- ASS alpha
						return 255 - tonumber( ass_r_a:sub( 3, 4 ), 16 )
					elseif ass_r_a:find( "^&H%x%x%x%x%x%x&$" ) then	-- ASS color
						return tonumber( ass_r_a:sub( 7, 8 ), 16 ), tonumber( ass_r_a:sub( 5, 6 ), 16 ), tonumber( ass_r_a:sub( 3, 4 ), 16 )
					elseif ass_r_a:find( "^&H%x%x%x%x%x%x%x%x$" ) then	-- ASS color+alpha (style)
						return tonumber( ass_r_a:sub( 9, 10 ), 16 ), tonumber( ass_r_a:sub( 7, 8 ), 16 ), tonumber( ass_r_a:sub( 5, 6 ), 16 ), 255 - tonumber( ass_r_a:sub( 3, 4 ), 16 )
					else
						error( "invalid string" )
					end
				else
					error( "color, alpha or color+alpha as numeric or ASS expected", 2 )
				end
			end,
			
			interpolate_coloralpha = function( pct, ... )
				-- Interpolates between two ASS colors &/+ alphas
				local args = { ... }
				args.n = #args
				if type( pct ) ~= "number" or pct < 0 or pct > 1 or args.n < 2 then
					error( "progress and at least two ASS values of same type (color, alpha or color+alpha) expected", 2 )
				end
				for i = 1, args.n do
					if type( args[ i ] ) ~= "string" then
						error( "ASS values must be strings", 2 )
					end
				end
				local i = math.min( 1 + floor( pct * (args.n - 1) ), args.n - 1 )
				local success1, ass_r_a1, g1, b1, a1 = pcall( ke4.ass.convert_coloralpha, args[ i ] )
				local success2, ass_r_a2, g2, b2, a2 = pcall( ke4.ass.convert_coloralpha, args[ i + 1 ] )
				if not success1 or not success2 then
					error( "invalid ASS value(s)", 2 )
				end
				local min_pct, max_pct = (i - 1) / (args.n - 1), i / (args.n - 1)
				local inner_pct = (pct - min_pct) / (max_pct - min_pct)
				if a1 and a2 then	-- Color + alpha
					return ke4.ass.convert_coloralpha( ass_r_a1 + (ass_r_a2 - ass_r_a1) * inner_pct, g1 + (g2 - g1) * inner_pct, b1 + (b2 - b1) * inner_pct, a1 + (a2 - a1) * inner_pct )
				elseif b1 and not a1 and b2 and not a2 then	-- Color
					return ke4.ass.convert_coloralpha( ass_r_a1 + (ass_r_a2 - ass_r_a1) * inner_pct, g1 + (g2 - g1) * inner_pct, b1 + (b2 - b1) * inner_pct )
				elseif not g1 and not g2 then	-- Alpha
					return ke4.ass.convert_coloralpha( ass_r_a1 + (ass_r_a2 - ass_r_a1) * inner_pct )
				else
					error( "ASS values must be the same type", 2 )
				end
			end,
			
			create_parser = function( ass_text )
				-- Creates an ASS parser
				if ass_text ~= nil and type( ass_text ) ~= "string" then
					error( "optional string expected", 2 )
				end
				local section = ""
				local meta = { wrap_style = 0, scaled_border_and_shadow = true, play_res_x = 0, play_res_y = 0 }
				local styles = { }
				local dialogs = { n = 0 }
				local obj = {
					parse_line = function( line )
						if type( line ) ~= "string" then
							error( "string expected", 2 )
						end
						if line:find( "^%[.-%]$" ) then	-- Define section
							section = line:sub( 2, -2 )
							return true
						elseif section == "Script Info" then	-- Meta
							if line:find( "^WrapStyle: %d$" ) then
								meta.wrap_style = tonumber( line:sub( 12 ) )
								return true
							elseif line:find( "^ScaledBorderAndShadow: %l+$" ) then
								local value = line:sub( 24 )
								if value == "yes" or value == "no" then
									meta.scaled_border_and_shadow = value == "yes"
									return true
								end
							elseif line:find( "^PlayResX: %d+$" ) then
								meta.play_res_x = tonumber( line:sub( 11 ) )
								return true
							elseif line:find( "^PlayResY: %d+$" ) then
								meta.play_res_y = tonumber( line:sub( 11 ) )
								return true
							end
						elseif section == "V4+ Styles" then	-- Styles
							local name, fontname, fontsize, color1, color2, color3, color4,
									bold, italic, underline, strikeout, scale_x, scale_y, spacing, angle, border_style,
									outline, shadow, alignment, margin_l, margin_r, margin_v, encoding =
									line:match( "^Style: (.-),(.-),(%d+),(&H%x%x%x%x%x%x%x%x),(&H%x%x%x%x%x%x%x%x),(&H%x%x%x%x%x%x%x%x),(&H%x%x%x%x%x%x%x%x),(%-?[01]),(%-?[01]),(%-?[01]),(%-?[01]),(%d+%.?%d*),(%d+%.?%d*),(%-?%d+%.?%d*),(%-?%d+%.?%d*),([13]),(%d+%.?%d*),(%d+%.?%d*),([1-9]),(%d+%.?%d*),(%d+%.?%d*),(%d+%.?%d*),(%d+)$" )
							if encoding and tonumber( encoding ) <= 255 then
								local style = {
									fontname = fontname,
									fontsize = tonumber( fontsize ),
									bold = bold == "-1",
									italic = italic == "-1",
									underline = underline == "-1",
									strikeout = strikeout == "-1",
									scale_x = tonumber( scale_x ),
									scale_y = tonumber( scale_y ),
									spacing = tonumber( spacing ),
									angle = tonumber( angle ),
									border_style = border_style == "3",
									outline = tonumber( outline ),
									shadow = tonumber( shadow ),
									alignment = tonumber( alignment ),
									margin_l = tonumber( margin_l ),
									margin_r = tonumber( margin_r ),
									margin_v = tonumber( margin_v ),
									encoding = tonumber( encoding )
								}
								local r, g, b, a = ke4.ass.convert_coloralpha( color1 )
								style.color1 = ke4.ass.convert_coloralpha( r, g, b )
								style.alpha1 = ke4.ass.convert_coloralpha( a )
								r, g, b, a = ke4.ass.convert_coloralpha( color2 )
								style.color2 = ke4.ass.convert_coloralpha( r, g, b )
								style.alpha2 = ke4.ass.convert_coloralpha( a )
								r, g, b, a = ke4.ass.convert_coloralpha( color3 )
								style.color3 = ke4.ass.convert_coloralpha( r, g, b )
								style.alpha3 = ke4.ass.convert_coloralpha( a )
								r, g, b, a = ke4.ass.convert_coloralpha( color4 )
								style.color4 = ke4.ass.convert_coloralpha( r, g, b )
								style.alpha4 = ke4.ass.convert_coloralpha( a )
								styles[ name ] = style
								return true
							end
						elseif section == "Events" then	-- Dialogs
							local typ, layer, start_time, end_time, style, actor, margin_l, margin_r, margin_v, effect, text =
									line:match( "^(.-): (%d+),(%d:%d%d:%d%d%.%d%d),(%d:%d%d:%d%d%.%d%d),(.-),(.-),(%d+%.?%d*),(%d+%.?%d*),(%d+%.?%d*),(.-),(.*)$" )
							if text and ( typ == "Dialogue" or typ == "Comment" ) then
								dialogs.n = dialogs.n + 1
								dialogs[ dialogs.n ] = {
									comment = typ == "Comment",
									layer = tonumber( layer ),
									start_time = ke4.ass.convert_time( start_time ),
									end_time = ke4.ass.convert_time( end_time ),
									style = style,
									actor = actor,
									margin_l = tonumber( margin_l ),
									margin_r = tonumber( margin_r ),
									margin_v = tonumber( margin_v ),
									effect = effect,
									text = text
								}
								return true
							end
						end
						-- Nothing parsed
						return false
					end,
					meta = function( )
						return ke4.table.copy( meta )
					end,
					styles = function( )
						return ke4.table.copy( styles )
					end,
					dialogs = function( extended )
						if extended ~= nil and type( extended ) ~= "boolean" then
							error( "optional extension flag expected" )
						end
						if extended then
							local function text_sizes( text, style )
								local font = ke4.decode.create_font( style.fontname, style.bold, style.italic, style.underline, style.strikeout, style.fontsize, style.scale_x / 100, style.scale_y / 100, style.spacing )
								local extents, metrics = font.text_extents( text ), font.metrics( )
								return extents.width, extents.height, metrics.ascent, metrics.descent, metrics.internal_leading, metrics.external_leading
							end
							if not pcall( text_sizes, "Test", { fontname = "Arial", fontsize = 10, bold = false, italic = false, underline = false, strikeout = false, scale_x = 100, scale_y = 100, spacing = 0 } ) then	-- Fonts aren't supported/available?
								text_sizes = nil
							end
							local dialogs, dialog_styles, dialog, style_dialogs = ke4.table.copy( dialogs ), { }
							local space_width
							for i = 1, dialogs.n do
								dialog = dialogs[ i ]
								style_dialogs = dialog_styles[ dialog.style ]
								if not style_dialogs then
									style_dialogs = { n = 0 }
									dialog_styles[ dialog.style ] = style_dialogs
								end
								style_dialogs.n = style_dialogs.n + 1
								style_dialogs[ style_dialogs.n ] = dialog
								dialog.i = i
								dialog.duration = dialog.end_time - dialog.start_time
								dialog.mid_time = dialog.start_time + dialog.duration / 2
								dialog.styleref = styles[ dialog.style ]
								dialog.text_stripped = dialog.text:gsub( "{.-}", "" )
								if text_sizes and dialog.styleref then
									dialog.width, dialog.height, dialog.ascent, dialog.descent, dialog.internal_leading, dialog.external_leading = text_sizes( dialog.text_stripped, dialog.styleref )
									if meta.play_res_x > 0 and meta.play_res_y > 0 then
										if (dialog.styleref.alignment - 1) % 3 == 0 then
											dialog.left = dialog.margin_l ~= 0 and dialog.margin_l or dialog.styleref.margin_l
											dialog.center = dialog.left + dialog.width / 2
											dialog.right = dialog.left + dialog.width
											dialog.x = dialog.left
										elseif (dialog.styleref.alignment - 2) % 3 == 0 then
											dialog.left = meta.play_res_x / 2 - dialog.width / 2
											dialog.center = dialog.left + dialog.width / 2
											dialog.right = dialog.left + dialog.width
											dialog.x = dialog.center
										else
											dialog.left = meta.play_res_x - (dialog.margin_r ~= 0 and dialog.margin_r or dialog.styleref.margin_r) - dialog.width
											dialog.center = dialog.left + dialog.width / 2
											dialog.right = dialog.left + dialog.width
											dialog.x = dialog.right
										end
										if dialog.styleref.alignment > 6 then
											dialog.top = dialog.margin_v ~= 0 and dialog.margin_v or dialog.styleref.margin_v
											dialog.middle = dialog.top + dialog.height / 2
											dialog.bottom = dialog.top + dialog.height
											dialog.y = dialog.top
										elseif dialog.styleref.alignment > 3 then
											dialog.top = meta.play_res_y / 2 - dialog.height / 2
											dialog.middle = dialog.top + dialog.height / 2
											dialog.bottom = dialog.top + dialog.height
											dialog.y = dialog.middle
										else
											dialog.top = meta.play_res_y - (dialog.margin_v ~= 0 and dialog.margin_v or dialog.styleref.margin_v) - dialog.height
											dialog.middle = dialog.top + dialog.height / 2
											dialog.bottom = dialog.top + dialog.height
											dialog.y = dialog.bottom
										end
									end
									space_width = text_sizes( " ", dialog.styleref )
								end
								dialog.text_chunked = { n = 0 }
								do
									local chunk_start, chunk_end = dialog.text:find( "{.-}" )
									if not chunk_start then
										dialog.text_chunked = { n = 1, { tags = "", text = dialog.text } }
									else
										if chunk_start ~= 1 then
											dialog.text_chunked.n = dialog.text_chunked.n + 1
											dialog.text_chunked[ dialog.text_chunked.n ] = { tags = "", text = dialog.text:sub( 1, chunk_start - 1 ) }
										end
										local chunk2_start, chunk2_end
										repeat
											chunk2_start, chunk2_end = dialog.text:find( "{.-}", chunk_end + 1 )
											dialog.text_chunked.n = dialog.text_chunked.n + 1
											dialog.text_chunked[ dialog.text_chunked.n ] = { tags = dialog.text:sub( chunk_start + 1, chunk_end - 1 ), text = dialog.text:sub( chunk_end + 1, chunk2_start and chunk2_start - 1 or -1 ) }
											chunk_start, chunk_end = chunk2_start, chunk2_end
										until not chunk_start
									end
								end
								dialog.syls = { n = 0 }
								do
									local last_time, text_chunk, pretags, kdur, posttags, syl = 0
									for i=1, dialog.text_chunked.n do
										text_chunk = dialog.text_chunked[ i ]
										pretags, kdur, posttags = text_chunk.tags:match( "(.-)\\[kK][of]?(%d+)(.*)" )
										if posttags then
											syl = {
												i = dialog.syls.n + 1,
												start_time = last_time,
												mid_time = last_time + kdur * 10 / 2,
												end_time = last_time + kdur * 10,
												duration = kdur * 10,
												tags = pretags .. posttags
											}
											syl.prespace, syl.text, syl.postspace = text_chunk.text:match( "(%s*)(%S*)(%s*)" )
											syl.prespace, syl.postspace = syl.prespace:len( ), syl.postspace:len( )
											if text_sizes and dialog.styleref then
												syl.width, syl.height, syl.ascent, syl.descent, syl.internal_leading, syl.external_leading = text_sizes( syl.text, dialog.styleref )
											end
											last_time = syl.end_time
											dialog.syls.n = dialog.syls.n + 1
											dialog.syls[ dialog.syls.n ] = syl
										else
											dialog.syls = { n = 0 }
											break
										end
									end
									if dialog.syls.n > 0 and dialog.syls[ 1 ].width and meta.play_res_x > 0 and meta.play_res_y > 0 then
										if dialog.styleref.alignment > 6 or dialog.styleref.alignment < 4 then
											local cur_x = dialog.left
											for i=1, dialog.syls.n do
												syl = dialog.syls[ i ]
												cur_x = cur_x + syl.prespace * space_width
												syl.left = cur_x
												syl.center = syl.left + syl.width / 2
												syl.right = syl.left + syl.width
												syl.x = (dialog.styleref.alignment - 1) % 3 == 0 and syl.left or
														(dialog.styleref.alignment - 2) % 3 == 0 and syl.center or
														syl.right
												cur_x = cur_x + syl.width + syl.postspace * space_width
												syl.top = dialog.top
												syl.middle = dialog.middle
												syl.bottom = dialog.bottom
												syl.y = dialog.y
											end
										else
											local max_width, sum_height = 0, 0
											for i = 1, dialog.syls.n do
												syl = dialog.syls[ i ]
												max_width = math.max( max_width, syl.width )
												sum_height = sum_height + syl.height
											end
											local cur_y, x_fix = meta.play_res_y / 2 - sum_height / 2
											for i = 1, dialog.syls.n do
												syl = dialog.syls[ i ]
												x_fix = (max_width - syl.width) / 2
												if dialog.styleref.alignment == 4 then
													syl.left = dialog.left + x_fix
													syl.center = syl.left + syl.width / 2
													syl.right = syl.left + syl.width
													syl.x = syl.left
												elseif dialog.styleref.alignment == 5 then
													syl.left = meta.play_res_x / 2 - syl.width / 2
													syl.center = syl.left + syl.width / 2
													syl.right = syl.left + syl.width
													syl.x = syl.center
												else -- dialog.styleref.alignment == 6
													syl.left = dialog.right - syl.width - x_fix
													syl.center = syl.left + syl.width / 2
													syl.right = syl.left + syl.width
													syl.x = syl.right
												end
												-- Vertical position
												syl.top = cur_y
												syl.middle = syl.top + syl.height / 2
												syl.bottom = syl.top + syl.height
												syl.y = syl.middle
												cur_y = cur_y + syl.height
											end
										end
									end
								end
								dialog.words = { n = 0 }
								do
									local word
									for prespace, word_text, postspace in dialog.text_stripped:gmatch( "(%s*)(%S+)(%s*)" ) do
										word = {
											i = dialog.words.n + 1,
											start_time = dialog.start_time,
											mid_time = dialog.mid_time,
											end_time = dialog.end_time,
											duration = dialog.duration,
											text = word_text,
											prespace = prespace:len( ),
											postspace = postspace:len( )
										}
										if text_sizes and dialog.styleref then
											word.width, word.height, word.ascent, word.descent, word.internal_leading, word.external_leading = text_sizes( word.text, dialog.styleref )
										end
										dialog.words.n = dialog.words.n + 1
										dialog.words[ dialog.words.n ] = word
									end
									if dialog.words.n > 0 and dialog.words[ 1 ].width and meta.play_res_x > 0 and meta.play_res_y > 0 then
										if dialog.styleref.alignment > 6 or dialog.styleref.alignment < 4 then
											local cur_x = dialog.left
											for i = 1, dialog.words.n do
												word = dialog.words[ i ]
												cur_x = cur_x + word.prespace * space_width
												word.left = cur_x
												word.center = word.left + word.width / 2
												word.right = word.left + word.width
												word.x = (dialog.styleref.alignment - 1) % 3 == 0 and word.left or
														(dialog.styleref.alignment - 2) % 3 == 0 and word.center or
														word.right
												cur_x = cur_x + word.width + word.postspace * space_width
												word.top = dialog.top
												word.middle = dialog.middle
												word.bottom = dialog.bottom
												word.y = dialog.y
											end
										else
											local max_width, sum_height = 0, 0
											for i = 1, dialog.words.n do
												word = dialog.words[ i ]
												max_width = math.max( max_width, word.width )
												sum_height = sum_height + word.height
											end
											local cur_y, x_fix = meta.play_res_y / 2 - sum_height / 2
											for i = 1, dialog.words.n do
												word = dialog.words[ i ]
												x_fix = (max_width - word.width) / 2
												if dialog.styleref.alignment == 4 then
													word.left = dialog.left + x_fix
													word.center = word.left + word.width / 2
													word.right = word.left + word.width
													word.x = word.left
												elseif dialog.styleref.alignment == 5 then
													word.left = meta.play_res_x / 2 - word.width / 2
													word.center = word.left + word.width / 2
													word.right = word.left + word.width
													word.x = word.center
												else -- dialog.styleref.alignment == 6
													word.left = dialog.right - word.width - x_fix
													word.center = word.left + word.width / 2
													word.right = word.left + word.width
													word.x = word.right
												end
												word.top = cur_y
												word.middle = word.top + word.height / 2
												word.bottom = word.top + word.height
												word.y = word.middle
												cur_y = cur_y + word.height
											end
										end
									end
								end
								dialog.chars = { n = 0 }
								do
									local char, char_index, syl, word
									for _, char_text in ke4.utf8.chars( dialog.text_stripped ) do
										char = {
											i = dialog.chars.n + 1,
											start_time = dialog.start_time,
											mid_time = dialog.mid_time,
											end_time = dialog.end_time,
											duration = dialog.duration,
											text = char_text
										}
										char_index = 0
										for i = 1, dialog.syls.n do
											syl = dialog.syls[ i ]
											for _ in ke4.utf8.chars( format( "%s%s%s", string.rep( " ", syl.prespace ), syl.text, string.rep( " ", syl.postspace ) ) ) do
												char_index = char_index + 1
												if char_index == char.i then
													char.syl_i = syl.i
													char.start_time = syl.start_time
													char.mid_time = syl.mid_time
													char.end_time = syl.end_time
													char.duration = syl.duration
													goto syl_reference_found
												end
											end
										end
										::syl_reference_found::
										char_index = 0
										for i = 1, dialog.words.n do
											word = dialog.words[ i ]
											for _ in ke4.utf8.chars( format( "%s%s%s", string.rep( " ", word.prespace ), word.text, string.rep( " ", word.postspace ) ) ) do
												char_index = char_index + 1
												if char_index == char.i then
													char.word_i = word.i
													goto word_reference_found
												end
											end
										end
										::word_reference_found::
										if text_sizes and dialog.styleref then
											char.width, char.height, char.ascent, char.descent, char.internal_leading, char.external_leading = text_sizes( char.text, dialog.styleref )
										end
										dialog.chars.n = dialog.chars.n + 1
										dialog.chars[ dialog.chars.n ] = char
									end
									if dialog.chars.n > 0 and dialog.chars[ 1 ].width and meta.play_res_x > 0 and meta.play_res_y > 0 then
										if dialog.styleref.alignment > 6 or dialog.styleref.alignment < 4 then
											local cur_x = dialog.left
											for i = 1, dialog.chars.n do
												char = dialog.chars[ i ]
												char.left = cur_x
												char.center = char.left + char.width / 2
												char.right = char.left + char.width
												char.x = (dialog.styleref.alignment - 1) % 3 == 0 and char.left or
														(dialog.styleref.alignment - 2) % 3 == 0 and char.center or
														char.right
												cur_x = cur_x + char.width
												char.top = dialog.top
												char.middle = dialog.middle
												char.bottom = dialog.bottom
												char.y = dialog.y
											end
										else
											local max_width, sum_height = 0, 0
											for i = 1, dialog.chars.n do
												char = dialog.chars[ i ]
												max_width = math.max( max_width, char.width )
												sum_height = sum_height + char.height
											end
											local cur_y, x_fix = meta.play_res_y / 2 - sum_height / 2
											for i = 1, dialog.chars.n do
												char = dialog.chars[ i ]
												x_fix = (max_width - char.width) / 2
												if dialog.styleref.alignment == 4 then
													char.left = dialog.left + x_fix
													char.center = char.left + char.width / 2
													char.right = char.left + char.width
													char.x = char.left
												elseif dialog.styleref.alignment == 5 then
													char.left = meta.play_res_x / 2 - char.width / 2
													char.center = char.left + char.width / 2
													char.right = char.left + char.width
													char.x = char.center
												else -- dialog.styleref.alignment == 6
													char.left = dialog.right - char.width - x_fix
													char.center = char.left + char.width / 2
													char.right = char.left + char.width
													char.x = char.right
												end
												char.top = cur_y
												char.middle = char.top + char.height / 2
												char.bottom = char.top + char.height
												char.y = char.middle
												cur_y = cur_y + char.height
											end
										end
									end
								end
							end
							for _, dialogs in pairs( dialog_styles ) do
								table.sort( dialogs, function( dialog1, dialog2 ) return dialog1.start_time <= dialog2.start_time end )
								for i = 1, dialogs.n do
									dialog = dialogs[ i ]
									dialog.leadin = i == 1 and 1000.1 or dialog.start_time - dialogs[ i - 1 ].end_time
									dialog.leadout = i == dialogs.n and 1000.1 or dialogs[ i + 1 ].start_time - dialog.end_time
								end
							end
							return dialogs
						else
							return ke4.table.copy( dialogs )
						end
					end
				}
				if ass_text then
					for line in ke4.algorithm.lines( ass_text ) do
						obj.parse_line( line )	-- no errors possible
					end
				end
				return obj
			end,
			
			gradient = function( Size, Valor, ... )
				--retorna la interpolación de los valores ingresados
				--interpola números, shapes, clips vectoriales, clips rectangulares, colores y transparencias
				local valor_i = Valor or 1
				local Tags, algorithm
				if type( Valor ) == "table" then
					valor_i = Valor[ 1 ] or 1
					Tags = Valor[ 2 ] or ""
					algorithm = Valor[ 3 ] or nil
				end
				local valors = { ... }
				if type( ... ) == "table" then
					valors = ...
				end
				return ke4.table.ipol( valors, Size, Tags, algorithm )[ valor_i ]
			end, --{\an5\pos($x,$y)\1c!_G.ke4.ass.gradient( line.kara.n, syl.i, "&HFFFFFF&", "&HFFFF00&", "&HFF00FF&", "&H00FFFF&" )!}
			
			gradient_line = function( Text, ... )
				local Text = Text:gsub( "%b{}", "" )				--retira las etiquetas del texto :D
				local tbl_space = ke4.table.space( Text )			--tabla con las posiciones de los espacios del texto
				Text = Text:gsub( " ", "" )							--retira los espacios del texto
				local tbl_chars = ke4.table.string( Text, 1 )		--tabla con los caracteres del texto
				local Size = #tbl_chars								--tamaño de la tabla de gradientes
				local Values = { ... }								--tabla de valores de tags a interpolar
				if type( Values[ 1 ][ 1 ] ) == "table" then			--por si es una tabla que contenga todas las tablas
					Values = ...
				end
				local tbl_tagsi, Tags, algorithm = { }, ""
				for i = 1, #Values do
					Tags = Values[ i ][ 1 ] 						--tag a degradar
					table.remove( Values[ i ], 1 )					--se retira el tag de la tabla de valores
					if type( Values[ i ][ 1 ] ) == "table" then		--{ "tag", { algorithm }, val_1, val_2, val_3, ... }
						algorithm = Values[ i ][ 1 ][ 1 ]			--asigna el valor del algoritmo de interpolación
						table.remove( Values[ i ], 1 )				--elimina el "algorithm" de la tabla
					end
					tbl_tagsi[ i ] = ke4.table.ipol( Values[ i ], Size, Tags, algorithm ) -- aplica las interpolaciones
				end
				local tbl_gradi = ke4.table.concat3( tbl_tagsi )	--concatena todas las interpolaciones en una tabla
				for i = 1, #tbl_chars do							--aplica los tags a cada char del texto
					tbl_chars[ i ] = format( "{%s}%s", tbl_gradi[ i ], tbl_chars[ i ] )
				end
				if #tbl_space > 0 then								--pregunta si sí existen espacios en el texto
					for i = 1, #tbl_space do						--agrega nuevamente los espacios
						table.insert( tbl_chars, tbl_space[ i ], " " )
					end
				end
				return table.concat( tbl_chars )					--reagrupa nuevamente el texto con los tags agregados
			end, --!_G.ke4.ass.gradient_line( line.text_stripped, { "\\1c", "&H00FF00&", "&HFFFF00&" } )!
			
			linefx = function( Table_Line, Parts, Complete, Real )
				-- xres, yres --------------------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				----------------------------------------
				local linefx_tbl = {
					[ "start_time" ]	= Table_Line.start_time,
					[ "end_time" ]		= Table_Line.end_time,
					[ "duration" ]		= Table_Line.duration,
					[ "dur" ]			= Table_Line.duration,
					[ "text" ]			= Table_Line.text,
					[ "text_stripped" ]	= Table_Line.text_stripped,
					[ "width" ]			= ke4.math.round( Table_Line.width, 3 ),
					[ "height" ]		= Table_Line.height,
					[ "left" ]			= ke4.math.round( Table_Line.left, 3 ),
					[ "bottom" ]		= Table_Line.bottom,
					[ "right" ]			= ke4.math.round( Table_Line.right, 3 ),
					[ "top" ]			= Table_Line.top,
					[ "x" ]				= Table_Line.x,
					[ "y" ]				= Table_Line.y,
					[ "center" ]		= Table_Line.center,
					[ "middle" ]		= Table_Line.middle,
					[ "descent" ]		= ke4.math.round( Table_Line.descent, 3 ),
					[ "i" ]				= Table_Line.i,
					[ "xres" ]			= xres,
					[ "yres" ]			= yres,
					[ "styleref" ]		= Table_Line.styleref,
					[ "margin_t" ]		= Table_Line.eff_margin_t,
					[ "margin_l" ]		= Table_Line.eff_margin_l,
					[ "margin_b" ]		= Table_Line.eff_margin_b,
					[ "margin_r" ]		= Table_Line.eff_margin_r,
					[ "style" ]			= Table_Line.style,
					[ "section" ]		= Table_Line.section,
					[ "effect" ]		= Table_Line.effect,
					[ "halign" ]		= Table_Line.halign,
					[ "class" ]			= Table_Line.class,
					[ "valign" ]		= Table_Line.valign,
					[ "hcenter" ]		= Table_Line.hcenter,
					[ "layer" ]			= Table_Line.layer,
					[ "actor" ]			= Table_Line.actor,
					[ "vcenter" ]		= Table_Line.vcenter,
					[ "comment" ]		= Table_Line.comment,
					[ "extlead" ]		= Table_Line.extlead,
					[ "raw" ]			= Table_Line.raw,
					----------------------------------------------
				}
				
				--------------------------------------------------------------------------------------
				if Complete then
					local function real_bt( String, Top )
						if Real then
							local txt_shape = ke4.text.to_shape( linefx_tbl.styleref, String )
							ke4.shape.info( txt_shape )
							local min_y, max_y = miny, maxy
							return Top + min_y, Top + max_y
						end
						return 0, 0
					end
					local syl_text, syl_dur = ke4.text.text2syl( linefx_tbl.text, linefx_tbl.duration )
					local char_text, char_dur = ke4.text.text2char( linefx_tbl.text, linefx_tbl.duration )
					local word_text, word_dur = ke4.text.text2word( linefx_tbl.text, linefx_tbl.duration )
					--------------------------------------------------------------------------------------
					
					------------▼ syl
					if #syl_text > 0 then
						linefx_tbl[ "syl" ] = { }
						linefx_tbl.syl.text = ""
						local syl_left_pos = linefx_tbl.left
						local syl_start_time = 0
						local syl_real_top, syl_real_bottom
						for i = 1, #syl_text do
							linefx_tbl.syl[ i ] = { }
							linefx_tbl.syl[ i ].text			= ke4.text.karaoke_true( syl_text )
																and syl_text[ i ]:gsub( "KEclip", " " )
																or format( "{\\k%s}%s", ke4.math.round( syl_dur[ i ] / 10 ), syl_text[ i ] ):gsub( "KEclip", " " )
							linefx_tbl.syl[ i ].text_stripped	= ke4.text.text2stripped( syl_text[ i ] )
							linefx_tbl.syl[ i ].text_raw		= linefx_tbl.syl[ i ].text:gsub( "KEclip", " " )
							linefx_tbl.syl[ i ].tags			= linefx_tbl.syl[ i ].text:match( "%b{}" ) or ""
							linefx_tbl.syl[ i ].text1			= ke4.text.remove_tags( syl_text[ i ] ):gsub( "KEfx", "" )
							linefx_tbl.syl[ i ].text2			= linefx_tbl.syl[ i ].text_stripped:gsub( "KEfx", "" )
							linefx_tbl.syl[ i ].width_t			= aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.syl[ i ].text1 )
							linefx_tbl.syl[ i ].width			= ke4.math.round( aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.syl[ i ].text2 ), 3 )
							linefx_tbl.syl[ i ].left			= ke4.math.round( syl_left_pos, 3 )
							linefx_tbl.syl[ i ].center			= ke4.math.round( syl_left_pos + linefx_tbl.syl[ i ].width / 2, 3 )
							linefx_tbl.syl[ i ].right			= ke4.math.round( syl_left_pos + linefx_tbl.syl[ i ].width, 3 )
							linefx_tbl.syl[ i ].top				= linefx_tbl.top
							linefx_tbl.syl[ i ].middle			= linefx_tbl.middle
							linefx_tbl.syl[ i ].bottom			= linefx_tbl.bottom
							linefx_tbl.syl[ i ].height			= linefx_tbl.height
							linefx_tbl.syl[ i ].dur				= syl_dur[ i ]
							linefx_tbl.syl[ i ].duration		= syl_dur[ i ]
							linefx_tbl.syl[ i ].start_time		= syl_start_time
							linefx_tbl.syl[ i ].end_time		= linefx_tbl.syl[ i ].start_time + linefx_tbl.syl[ i ].dur
							linefx_tbl.syl[ i ].mid_time		= linefx_tbl.syl[ i ].start_time + linefx_tbl.syl[ i ].dur / 2
							linefx_tbl.syl.text					= linefx_tbl.syl.text .. linefx_tbl.syl[ i ].text:gsub( "KEfx", "" )
							syl_left_pos 						= syl_left_pos + linefx_tbl.syl[ i ].width_t
							syl_start_time						= syl_start_time + linefx_tbl.syl[ i ].dur
							syl_real_top, syl_real_bottom		= real_bt( linefx_tbl.syl[ i ].text_stripped, linefx_tbl.top )
							linefx_tbl.syl[ i ].rtop			= syl_real_top
							linefx_tbl.syl[ i ].rbottom			= syl_real_bottom
						end
					end
					
					------------▼ char
					if #char_text > 0 then
						linefx_tbl[ "char" ] = { }
						linefx_tbl.char.text = ""
						local char_left_pos = linefx_tbl.left
						local char_start_time = 0
						local char_real_top, char_real_bottom
						for i = 1, #char_text do
							linefx_tbl.char[ i ] = { }
							linefx_tbl.char[ i ].text			= format( "{\\k%s}%s", ke4.math.round( char_dur[ i ] / 10 ), char_text[ i ] ):gsub( "KEclip", " " )
							linefx_tbl.char[ i ].text_stripped	= ke4.text.text2stripped( char_text[ i ] )
							linefx_tbl.char[ i ].text_raw		= linefx_tbl.char[ i ].text:gsub( "KEclip", " " )
							linefx_tbl.char[ i ].tags			= linefx_tbl.char[ i ].text:match( "%b{}" ) or ""
							linefx_tbl.char[ i ].text1			= ke4.text.remove_tags( char_text[ i ] ):gsub( "KEfx", "" )
							linefx_tbl.char[ i ].text2			= linefx_tbl.char[ i ].text_stripped:gsub( "KEfx", "" )
							linefx_tbl.char[ i ].width_t		= aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.char[ i ].text1 )
							linefx_tbl.char[ i ].width			= ke4.math.round( aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.char[ i ].text2 ), 3 )
							linefx_tbl.char[ i ].left			= ke4.math.round( char_left_pos, 3 )
							linefx_tbl.char[ i ].center			= ke4.math.round( char_left_pos + linefx_tbl.char[ i ].width / 2, 3 )
							linefx_tbl.char[ i ].right			= ke4.math.round( char_left_pos + linefx_tbl.char[ i ].width, 3 )
							linefx_tbl.char[ i ].top			= linefx_tbl.top
							linefx_tbl.char[ i ].middle			= linefx_tbl.middle
							linefx_tbl.char[ i ].bottom			= linefx_tbl.bottom
							linefx_tbl.char[ i ].height			= linefx_tbl.height
							linefx_tbl.char[ i ].dur			= char_dur[ i ]
							linefx_tbl.char[ i ].duration		= char_dur[ i ]
							linefx_tbl.char[ i ].start_time		= char_start_time
							linefx_tbl.char[ i ].end_time		= linefx_tbl.char[ i ].start_time + linefx_tbl.char[ i ].dur
							linefx_tbl.char[ i ].mid_time		= linefx_tbl.char[ i ].start_time + linefx_tbl.char[ i ].dur / 2
							linefx_tbl.char.text				= linefx_tbl.char.text .. linefx_tbl.char[ i ].text:gsub( "KEfx", "" )
							char_left_pos 						= char_left_pos + linefx_tbl.char[ i ].width_t
							char_start_time						= char_start_time + linefx_tbl.char[ i ].dur
							char_real_top, char_real_bottom		= real_bt( linefx_tbl.char[ i ].text_stripped, linefx_tbl.top )
							linefx_tbl.char[ i ].rtop			= char_real_top
							linefx_tbl.char[ i ].rbottom		= char_real_bottom
						end
						------------▼ char noblank
						linefx_tbl[ "char2" ] = { }
						for i = 1, #char_text do
							if linefx_tbl.char[ i ].text_stripped ~= " " then
								linefx_tbl.char2[ #linefx_tbl.char2 + 1 ] = linefx_tbl.char[ i ]
							end
						end
						linefx_tbl.char2.text = linefx_tbl.char.text
					end
					
					------------▼ word
					if #word_text > 0 then
						linefx_tbl[ "word" ] = { }
						linefx_tbl.word.text = ""
						local word_left_pos = linefx_tbl.left
						local word_start_time = 0
						local word_real_top, word_real_bottom
						for i = 1, #word_text do
							linefx_tbl.word[ i ] = { }
							linefx_tbl.word[ i ].text			= ke4.text.karaoke_true( word_text )
																and word_text[ i ]:gsub( "KEclip", " " )
																or format( "{\\k%s}%s", ke4.math.round( word_dur[ i ] / 10 ), word_text[ i ] ):gsub( "KEclip", " " )
							linefx_tbl.word[ i ].text_stripped	= ke4.text.text2stripped( word_text[ i ] )
							linefx_tbl.word[ i ].text_raw		= linefx_tbl.word[ i ].text:gsub( "KEclip", " " )
							linefx_tbl.word[ i ].tags			= linefx_tbl.word[ i ].text:match( "%b{}" ) or ""
							linefx_tbl.word[ i ].text1			= ke4.text.remove_tags( word_text[ i ] ):gsub( "KEfx", "" )
							linefx_tbl.word[ i ].text2			= linefx_tbl.word[ i ].text_stripped:gsub( "KEfx", "" )
							linefx_tbl.word[ i ].width_t		= aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.word[ i ].text1 )
							linefx_tbl.word[ i ].width			= ke4.math.round( aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.word[ i ].text2 ), 3 )
							linefx_tbl.word[ i ].left			= ke4.math.round( word_left_pos, 3 )
							linefx_tbl.word[ i ].center			= ke4.math.round( word_left_pos + linefx_tbl.word[ i ].width / 2, 3 )
							linefx_tbl.word[ i ].right			= ke4.math.round( word_left_pos + linefx_tbl.word[ i ].width, 3 )
							linefx_tbl.word[ i ].top			= linefx_tbl.top
							linefx_tbl.word[ i ].middle			= linefx_tbl.middle
							linefx_tbl.word[ i ].bottom			= linefx_tbl.bottom
							linefx_tbl.word[ i ].height			= linefx_tbl.height
							linefx_tbl.word[ i ].dur			= word_dur[ i ]
							linefx_tbl.word[ i ].duration		= word_dur[ i ]
							linefx_tbl.word[ i ].start_time		= word_start_time
							linefx_tbl.word[ i ].end_time		= linefx_tbl.word[ i ].start_time + linefx_tbl.word[ i ].dur
							linefx_tbl.word[ i ].mid_time		= linefx_tbl.word[ i ].start_time + linefx_tbl.word[ i ].dur / 2
							linefx_tbl.word.text				= linefx_tbl.word.text .. linefx_tbl.word[ i ].text:gsub( "KEfx", "" )
							word_left_pos 						= word_left_pos + linefx_tbl.word[ i ].width_t
							word_start_time						= word_start_time + linefx_tbl.word[ i ].dur
							word_real_top, word_real_bottom		= real_bt( linefx_tbl.word[ i ].text_stripped, linefx_tbl.top )
							linefx_tbl.word[ i ].rtop			= word_real_top
							linefx_tbl.word[ i ].rbottom		= word_real_bottom
						end
					end
					
					------------▼ part
					local Parts = Parts or 3
					local p_txt, p_dur, p_cen, p_wid, p_lef, p_rig = ke4.text.text2part( linefx_tbl.styleref, linefx_tbl.text_stripped, linefx_tbl.duration, linefx_tbl.left, Parts )
					if #p_txt > 0 then
						linefx_tbl[ "part" ] = { }
						linefx_tbl.part.text = ""
						local part_start_time = 0
						local part_real_top, part_real_bottom
						for i = 1, #p_txt do
							linefx_tbl.part[ i ] = { }
							linefx_tbl.part[ i ].text			= format( "{\\k%s}%s", ke4.math.round( p_dur[ i ] / 10 ), p_txt[ i ] )
							linefx_tbl.part[ i ].text_stripped	= p_txt[ i ]
							linefx_tbl.part[ i ].tags			= format( "{\\k%s}", ke4.math.round( p_dur[ i ] / 10 ) )
							linefx_tbl.part[ i ].width			= p_wid[ i ]
							linefx_tbl.part[ i ].left			= p_lef[ i ]
							linefx_tbl.part[ i ].center			= p_cen[ i ]
							linefx_tbl.part[ i ].right			= p_rig[ i ]
							linefx_tbl.part[ i ].top			= linefx_tbl.top
							linefx_tbl.part[ i ].middle			= linefx_tbl.middle
							linefx_tbl.part[ i ].bottom			= linefx_tbl.bottom
							linefx_tbl.part[ i ].height			= linefx_tbl.height
							linefx_tbl.part[ i ].dur			= p_dur[ i ]
							linefx_tbl.part[ i ].duration		= p_dur[ i ]
							linefx_tbl.part[ i ].start_time		= part_start_time
							linefx_tbl.part[ i ].end_time		= linefx_tbl.part[ i ].start_time + linefx_tbl.part[ i ].dur
							linefx_tbl.part[ i ].mid_time		= linefx_tbl.part[ i ].start_time + linefx_tbl.part[ i ].dur / 2
							linefx_tbl.part.text				= linefx_tbl.part.text .. linefx_tbl.part[ i ].text:gsub( "KEfx", "" )
							part_start_time						= part_start_time + linefx_tbl.part[ i ].dur
							part_real_top, part_real_bottom		= real_bt( linefx_tbl.part[ i ].text_stripped, linefx_tbl.top )
							linefx_tbl.part[ i ].rtop			= part_real_top
							linefx_tbl.part[ i ].rbottom		= part_real_bottom
						end
					end
					
					------------▼ romaji
					linefx_tbl[ "roma" ] = { }
					linefx_tbl.roma.text = ""
					local linefx_roma_wdth = 0
					local roma_start = 0
					local roma_real_top, roma_real_bottom
					for i = 1, #syl_text do
						linefx_tbl.roma[ i ]				= { }
						linefx_tbl.roma[ i ].tag			= syl_text[ i ]:match( "%{.-%}" ) or ""
						linefx_tbl.roma[ i ].text			= linefx_tbl.roma[ i ].tag:gsub( "KEclip", " " ) .. ke4.text.kana2romaji( ke4.text.remove_tags( syl_text[ i ] ) )
						linefx_tbl.roma[ i ].tags			= linefx_tbl.roma[ i ].text:match( "%b{}" )
						linefx_tbl.roma[ i ].text_raw		= linefx_tbl.roma[ i ].text:gsub( "KEclip", " " )
						linefx_tbl.roma[ i ].text_stripped	= ke4.text.text2stripped( linefx_tbl.roma[ i ].text )
						linefx_tbl.roma[ i ].text1			= ke4.text.remove_tags( linefx_tbl.roma[ i ].text ):gsub( "KEfx", "" )
						linefx_tbl.roma[ i ].text2			= linefx_tbl.roma[ i ].text_stripped:gsub( "KEfx", "" )
						linefx_tbl.roma[ i ].width_t		= aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.roma[ i ].text1 )
						linefx_tbl.roma[ i ].width			= ke4.math.round( aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.roma[ i ].text2 ), 3 )
						linefx_tbl.roma.text				= linefx_tbl.roma.text .. linefx_tbl.roma[ i ].text:gsub( "KEfx", "" )
						linefx_roma_wdth					= linefx_roma_wdth + linefx_tbl.roma[ i ].width_t
						roma_real_top, roma_real_bottom		= real_bt( linefx_tbl.roma[ i ].text_stripped, linefx_tbl.top )
						linefx_tbl.roma[ i ].rtop			= roma_real_top
						linefx_tbl.roma[ i ].rbottom		= roma_real_bottom
					end
					local options_lft_roma = {
						[ 1 ] = linefx_tbl.styleref.margin_l,
						[ 2 ] = (xres + linefx_tbl.styleref.margin_l - linefx_tbl.styleref.margin_r) / 2 - linefx_roma_wdth / 2,
						[ 3 ] = xres - linefx_tbl.styleref.margin_r - linefx_roma_wdth
					}
					local roma_left = options_lft_roma[ (linefx_tbl.styleref.align - 1) % 3 + 1 ]
					for i = 1, #syl_text do
						linefx_tbl.roma[ i ].left			= ke4.math.round( roma_left, 3 )
						linefx_tbl.roma[ i ].center			= ke4.math.round( roma_left + linefx_tbl.roma[ i ].width / 2, 3 )
						linefx_tbl.roma[ i ].right			= ke4.math.round( roma_left + linefx_tbl.roma[ i ].width, 3 )
						linefx_tbl.roma[ i ].top			= linefx_tbl.top
						linefx_tbl.roma[ i ].middle			= linefx_tbl.middle
						linefx_tbl.roma[ i ].bottom			= linefx_tbl.bottom
						linefx_tbl.roma[ i ].height			= linefx_tbl.height
						linefx_tbl.roma[ i ].duration		= syl_dur[ i ]
						linefx_tbl.roma[ i ].dur			= syl_dur[ i ]
						linefx_tbl.roma[ i ].start_time		= roma_start
						linefx_tbl.roma[ i ].end_time		= linefx_tbl.roma[ i ].start_time + linefx_tbl.roma[ i ].dur
						linefx_tbl.roma[ i ].mid_time		= linefx_tbl.roma[ i ].start_time + linefx_tbl.roma[ i ].dur / 2
						roma_left 							= roma_left + linefx_tbl.roma[ i ].width_t
						roma_start							= roma_start + linefx_tbl.roma[ i ].dur
					end
					
					------------▼ katakana
					linefx_tbl[ "kata" ] = { }
					linefx_tbl.kata.text = ""
					local linefx_kata_wdth = 0
					local kata_start = 0
					local kata_real_top, kata_real_bottom
					for i = 1, #syl_text do
						linefx_tbl.kata[ i ]				= { }
						linefx_tbl.kata[ i ].tag			= syl_text[ i ]:match( "%{.-%}" ) or ""
						linefx_tbl.kata[ i ].text			= linefx_tbl.kata[ i ].tag:gsub( "KEclip", " " ) .. ke4.text.syl2katakana( ke4.text.remove_tags( syl_text[ i ] ) )
						linefx_tbl.kata[ i ].tags			= linefx_tbl.kata[ i ].text:match( "%b{}" )
						linefx_tbl.kata[ i ].text_raw		= linefx_tbl.kata[ i ].text:gsub( "KEclip", " " )
						linefx_tbl.kata[ i ].text_stripped	= ke4.text.text2stripped( linefx_tbl.kata[ i ].text )
						linefx_tbl.kata[ i ].text1			= ke4.text.remove_tags( linefx_tbl.kata[ i ].text ):gsub( "KEfx", "" )
						linefx_tbl.kata[ i ].text2			= linefx_tbl.kata[ i ].text_stripped:gsub( "KEfx", "" )
						linefx_tbl.kata[ i ].width_t		= aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.kata[ i ].text1 )
						linefx_tbl.kata[ i ].width			= ke4.math.round( aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.kata[ i ].text2 ), 3 )
						linefx_tbl.kata.text				= linefx_tbl.kata.text .. linefx_tbl.kata[ i ].text:gsub( "KEfx", "" )
						linefx_kata_wdth					= linefx_kata_wdth + linefx_tbl.kata[ i ].width_t
						kata_real_top, kata_real_bottom		= real_bt( linefx_tbl.kata[ i ].text_stripped, linefx_tbl.top )
						linefx_tbl.kata[ i ].rtop			= kata_real_top
						linefx_tbl.kata[ i ].rbottom		= kata_real_bottom
					end
					local options_lft_kata = {
						[ 1 ] = linefx_tbl.styleref.margin_l,
						[ 2 ] = (xres + linefx_tbl.styleref.margin_l - linefx_tbl.styleref.margin_r) / 2 - linefx_kata_wdth / 2,
						[ 3 ] = xres - linefx_tbl.styleref.margin_r - linefx_kata_wdth
					}
					local kata_left = options_lft_kata[ (linefx_tbl.styleref.align - 1) % 3 + 1 ]
					for i = 1, #syl_text do
						linefx_tbl.kata[ i ].left			= ke4.math.round( kata_left, 3 )
						linefx_tbl.kata[ i ].center			= ke4.math.round( kata_left + linefx_tbl.kata[ i ].width / 2, 3 )
						linefx_tbl.kata[ i ].right			= ke4.math.round( kata_left + linefx_tbl.kata[ i ].width, 3 )
						linefx_tbl.kata[ i ].top			= linefx_tbl.top
						linefx_tbl.kata[ i ].middle			= linefx_tbl.middle
						linefx_tbl.kata[ i ].bottom			= linefx_tbl.bottom
						linefx_tbl.kata[ i ].height			= linefx_tbl.height
						linefx_tbl.kata[ i ].duration		= syl_dur[ i ]
						linefx_tbl.kata[ i ].dur			= syl_dur[ i ]
						linefx_tbl.kata[ i ].start_time		= kata_start
						linefx_tbl.kata[ i ].end_time		= linefx_tbl.kata[ i ].start_time + linefx_tbl.kata[ i ].dur
						linefx_tbl.kata[ i ].mid_time		= linefx_tbl.kata[ i ].start_time + linefx_tbl.kata[ i ].dur / 2
						kata_left 							= kata_left + linefx_tbl.kata[ i ].width_t
						kata_start							= kata_start + linefx_tbl.kata[ i ].dur
					end
					
					------------▼ hiragana
					linefx_tbl[ "hira" ] = { }
					linefx_tbl.hira.text = ""
					local linefx_hira_wdth = 0
					local hira_start = 0
					local hira_real_top, hira_real_bottom
					for i = 1, #syl_text do
						linefx_tbl.hira[ i ]				= { }
						linefx_tbl.hira[ i ].tag			= syl_text[ i ]:match( "%{.-%}" ) or ""
						linefx_tbl.hira[ i ].text			= linefx_tbl.hira[ i ].tag:gsub( "KEclip", " " ) .. ke4.text.syl2hiragana( ke4.text.remove_tags( syl_text[ i ] ) )
						linefx_tbl.hira[ i ].tags			= linefx_tbl.hira[ i ].text:match( "%b{}" )
						linefx_tbl.hira[ i ].text_raw		= linefx_tbl.hira[ i ].text:gsub( "KEclip", " " )
						linefx_tbl.hira[ i ].text_stripped	= ke4.text.text2stripped( linefx_tbl.hira[ i ].text )
						linefx_tbl.hira[ i ].text1			= ke4.text.remove_tags( linefx_tbl.hira[ i ].text ):gsub( "KEfx", "" )
						linefx_tbl.hira[ i ].text2			= linefx_tbl.hira[ i ].text_stripped:gsub( "KEfx", "" )
						linefx_tbl.hira[ i ].width_t		= aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.hira[ i ].text1 )
						linefx_tbl.hira[ i ].width			= ke4.math.round( aegisub.text_extents( linefx_tbl.styleref, linefx_tbl.hira[ i ].text2 ), 3 )
						linefx_tbl.hira.text				= linefx_tbl.hira.text .. linefx_tbl.hira[ i ].text:gsub( "KEfx", "" )
						linefx_hira_wdth					= linefx_hira_wdth + linefx_tbl.hira[ i ].width_t
						hira_real_top, hira_real_bottom		= real_bt( linefx_tbl.hira[ i ].text_stripped, linefx_tbl.top )
						linefx_tbl.hira[ i ].rtop			= hira_real_top
						linefx_tbl.hira[ i ].rbottom		= hira_real_bottom
					end
					local options_lft_hira = {
						[ 1 ] = linefx_tbl.styleref.margin_l,
						[ 2 ] = (xres + linefx_tbl.styleref.margin_l - linefx_tbl.styleref.margin_r) / 2 - linefx_hira_wdth / 2,
						[ 3 ] = xres - linefx_tbl.styleref.margin_r - linefx_hira_wdth
					}
					local hira_left = options_lft_hira[ (linefx_tbl.styleref.align - 1) % 3 + 1 ]
					for i = 1, #syl_text do
						linefx_tbl.hira[ i ].left			= ke4.math.round( hira_left, 3 )
						linefx_tbl.hira[ i ].center			= ke4.math.round( hira_left + linefx_tbl.hira[ i ].width / 2, 3 )
						linefx_tbl.hira[ i ].right			= ke4.math.round( hira_left + linefx_tbl.hira[ i ].width, 3 )
						linefx_tbl.hira[ i ].top			= linefx_tbl.top
						linefx_tbl.hira[ i ].middle			= linefx_tbl.middle
						linefx_tbl.hira[ i ].bottom			= linefx_tbl.bottom
						linefx_tbl.hira[ i ].height			= linefx_tbl.height
						linefx_tbl.hira[ i ].duration		= syl_dur[ i ]
						linefx_tbl.hira[ i ].dur			= syl_dur[ i ]
						linefx_tbl.hira[ i ].start_time		= hira_start
						linefx_tbl.hira[ i ].end_time		= linefx_tbl.hira[ i ].start_time + linefx_tbl.hira[ i ].dur
						linefx_tbl.hira[ i ].mid_time		= linefx_tbl.hira[ i ].start_time + linefx_tbl.hira[ i ].dur / 2
						hira_left 							= hira_left + linefx_tbl.hira[ i ].width_t
						hira_start							= hira_start + linefx_tbl.hira[ i ].dur
					end
				end
				return linefx_tbl
			end, --!_G.ke4.table.view( _G.ke4.ass.linefx( line, nil ) )!
			
		},
		
		-- Tag sublibrary
		tag = {
			oscill = function( DurTotal, DurDelay, ... )
				--DurDelay = DurDelay, or: DurDelay = { DurDelay, accel }, or: DurDelay = function
				--DurDelay = { DurDelay, accel, dilatation }, or: DurDelay = { { DurDelay, Dur_trans }, accel, dilatation }
				--DurDelay = { { DurDelay, Dur_trans }, accel, dilatation, offset_time }
				local time_ini, index_ii, time_tot, dur_tag1 = 0, 0, DurTotal, DurDelay
				local accel, dilat, offset_t, time_off, Tags = 1, 0, 0, 0, { ... }
				if type( DurTotal ) == "table" then
					time_ini =  DurTotal[ 1 ] or 0
					time_tot = (DurTotal[ 2 ] or 5000) - time_ini
					index_ii = (DurTotal[ 3 ] or 1) - 1
				elseif type( DurTotal ) == "function" then
					time_ini =  DurTotal( )[ 1 ] or 0
					time_tot = (DurTotal( )[ 2 ] or 5000) - time_ini
					index_ii = (DurTotal( )[ 3 ] or 1) - 1
				end
				----------------------------------------------------------
				local time_tot2 = time_tot
				if type( DurDelay ) == "table" then
					dur_tag1 = DurDelay[ 1 ]
					if type( DurDelay[ 1 ] ) == "table" then
						dur_tag1 = DurDelay[ 1 ][ 1 ]
					end
					if DurDelay[ 2 ] then
						accel = DurDelay[ 2 ]
					end
					if DurDelay[ 3 ] then
						dilat = DurDelay[ 3 ] / 2
					end
					if DurDelay[ 4 ] then
						offset_t = DurDelay[ 4 ]
						--tiempo que hay entre cada conjunto de tags oscills :D
					end
				end
				if type( ... ) == "function" then
					Tags = Tags[ 1 ]( )
				end
				if type( ... ) == "table" then
					Tags = ...
				end
				local time_i, time_f, tags_fx = 0, 1, ""
				local indicator, tag_osc = 1, ""
				if type( DurDelay ) == "number" then
					dur_tag1 = dur_tag1 - dilat
				elseif type( DurDelay ) == "function" then
					dur_tag1 = 0
				end
				local dur_func, dur_tag2 = 0, 0
				i = 0
				while time_tot > 0 do
					if type( DurDelay ) == "function" then
						dur_func = DurDelay( i )
						time_i = ke4.math.round( dur_tag2 + time_ini + time_off, 2 )
						time_f = ke4.math.round( time_i + dur_func, 2 )
					else
						time_i = ke4.math.round( dur_tag1 * i + time_ini + time_off, 2 )
						if type( DurDelay ) == "table" and
							type( DurDelay[ 1 ] ) == "table" then
							time_f = time_i + DurDelay[ 1 ][ 2 ]
						else
							time_f = ke4.math.round( (dur_tag1 + dilat) * (i + 1) + time_ini + time_off, 2 )
						end
					end
					indicator = #Tags - #Tags * ceil( (i + 1 + index_ii) / #Tags ) + i + 1 + index_ii
					if type( Tags[ indicator ] ) == "function" then
						tag_osc = Tags[ indicator ]( i )
					else
						if Tags[ indicator ]:gsub( "%b[]", "" ) == "" then
							Tags[ indicator ] = Tags[ indicator ]:sub( 2, -2 )
							local line = linefx[ ii ]
							func_oscll = loadstring(
								format( "return function( meta, line, x, y ) return %s end", Tags[ indicator ] )
							)( )
							Tags[ indicator ] = func_oscll( meta, line, x, y )
						end
						tag_osc = Tags[ indicator ]
					end
					---------------------------------------------------------------------------------
					if time_tot - dur_tag1 <= 0 then
						time_f = time_ini + time_tot2
						--hace que la última transfo termine en su tiempo exacto
					end
					---------------------------------------------------------------------------------
					tags_fx = tags_fx .. format( "\\t(%s,%s,%s,%s)", time_i, time_f, accel, tag_osc )
					tags_fx = tags_fx:gsub( "\\t%(%d+[%.%d]*%,%d+[%.%d]*%,[%d%.%,]*%)", "" )
					if type( DurDelay ) == "function" then
						dur_tag2 = dur_tag2 + dur_func
						dur_tag1 = dur_func
					else
						dur_tag1 = dur_tag1 + dilat
					end
					if (i + 1) % #Tags == 0 then
						if type( offset_t ) == "number" then
							time_off = time_off + offset_t
						elseif type( offset_t ) == "function" then
							time_off = time_off + offset_t( i )
						end
						--!_G.ke4.tag.oscill( { _G.ke4.math.R( 800 ), line.duration }, { { 100, 0 }, 1, 0, function( ) return _G.ke4.math.R( 500, 2000, 20 ) end }, "\\alpha&HFF&", "\\alpha&H00&" )!
					end
					time_tot = ke4.math.round( time_tot - dur_tag1, 2 )
					i = i + 1
				end
				tags_fx = tags_fx:gsub( "\\t%b()",
					function( capture )
						if type( tonumber( capture:match( "\\t%((%d+[%.%d]*)" ) ) ) == "number"
							and tonumber( capture:match( "\\t%((%d+[%.%d]*)" ) ) > time_ini + time_tot2 then
							return ""
						end
						return capture
					end
				)
				tags_fx = tags_fx:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" )
				tags_fx = ke4.string.i( tags_fx )
				return ke4.tag.oscill_dark( tags_fx )
			end,
			
			oscill_dark = function( String )
				local function redefinex( String )
					local String = String or ""
					--▼--------------------------------------------------------
					-- corvierte algunas convenciones en número
					local function tonumberx( String )
						-- xres, yres and ratio ----------------
						local xres, yres = aegisub.video_size( )
						if not xres then
							xres, yres = 1280, 720
						end
						ratio = xres / 1280
						-- frame_dur ---------------------------
						local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
						if msb then
							frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
						end
						----------------------------------------
						local String = String or ""
						local String_out = {
							[ 1 ] = "math%.bezier2move%b()",
							[ 2 ] = "text%.kana2romaji%b()",
							[ 3 ] = "%-?%d+[%.%d]*rnd[xyz]*",
							[ 4 ] = "%-?%d+[%.%d]*fr[sxyz]*",
							[ 5 ] = "%-?%d+[%.%d]*fs[cpvxy]*",
							[ 6 ] = "%-?%d+[%.%d]*f[ae]^*[xy]*",
						}
						local String_out_tbl = { }
						for i = 1, #String_out do
							-- protege segmentos del String de remplazos
							String = String:gsub( String_out[ i ],
								function( capture )
									table.insert( String_out_tbl, capture )
									return "▲"
								end
							)
						end
						-- multiplica una constante por el ratio o por frame_dur
						String = String:gsub( "(%-?%d+[%.%d]*)([rf]^*)",
							function( capture, variable )
								local varx = ratio
								if variable == "f" then
									varx = frame_dur
								end
								return tonumber( capture ) * varx
							end
						)
						for i = 1, #String_out_tbl do
							String = String:gsub( "▲", String_out_tbl[ i ], 1 )
						end
						-------------------------------------------------------------------------
						String = String:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
						-------------------------------------------------------------------------
						-- abreviatura de la función math.polar
						local polar_tag = {
							[ 1 ] = "(p[xy]^*)(%-?%d+[%.%d]*)d(%-?%d+[%.%d]*)",
							[ 2 ] = "(p[xy]^*)(%-?%d+[%.%d]*)d(%b())",
							[ 3 ] = "(p[xy]^*)(%b())d(%-?%d+[%.%d]*)",
							[ 4 ] = "(p[xy]^*)(%b())d(%b())",
						}
						for i = 1, #polar_tag do
							String = String:gsub( polar_tag[ i ],
								function( Pxy, Angle, Distance )
									local Return = "x"
									if Pxy:match( "py" ) then
										Return = "y"
									end
									local topolar = format( "ke4.math.polar( %s, %s, \"%s\" )", Angle, Distance, Return )
									local line = linefx[ ii ]
									if pcall( loadstring( format( "return function( meta, line, x, y ) return %s end", topolar ) ) ) then
										local fun_polar = loadstring( format( "return function( meta, line, x, y ) return %s end", topolar ) )( )
										if pcall( fun_polar ) then
											return fun_polar( meta, line, x, y )
										end
									end
									return Pxy .. Angle .. "d" .. Distance
								end
							)
						end --px45d100 = math.polar( 45, 100, "x" )
						-------------------------------------------------------------------------
						String = String:gsub( "\"(&H%x+&)\"", "%1" )
						-------------------------------------------------------------------------
						return String
					end
					String = tonumberx( String )
					--▼--------------------------------------------------------
					-- encierra la funcción R dentro de paréntesis
					String = String:gsub( "R[%w]*%b()", "(%1)" )
					--▼--------------------------------------------------------
					local function multi_alphas( String )
						local alphas = {
							[ 1 ] = "\\(%d%d+)a(%b())([%-%~]*)",
							[ 2 ] = "\\(%d%d+)a(&H%x+&)([%-%~]*)",
							[ 3 ] = "\\(%d%d+)a(%d+[%.%d]*)([%-%~]*)",
							------------------------------------------
							[ 4 ] = "\\(%d%d+)a(%b())([%-%~]^*%d+[%.%d]*)",
							[ 5 ] = "\\(%d%d+)a(&H%x+&)([%-%~]^*%d+[%.%d]*)",
							[ 6 ] = "\\(%d%d+)a(%d+[%.%d]*)([%-%~]^*%d+[%.%d]*)",
							------------------------------------------
							[ 7 ] = "\\(%d%d+)a(%b())([%-%~]^*%b())",
							[ 8 ] = "\\(%d%d+)a(&H%x+&)([%-%~]^*%b())",
							[ 9 ] = "\\(%d%d+)a(%d+[%.%d]*)([%-%~]^*%b())",
						}
						for i = 1, #alphas do
							String = String:gsub( alphas[ i ],
								function( nums, val, sign_val )
									local str_alp = ""
									local tbl_num = ke4.table.string( tostring( nums ) )
									for k = 1, #tbl_num do
										str_alp = str_alp .. format( "\\%sa%s", tbl_num[ k ], val .. sign_val )
									end
									return "@" .. str_alp .. "@"
								end		
							)
						end
						return String
					end --"\\34a200"
					--▼--------------------------------------------------------
					local function multi_colors( String )
						local String = String:gsub( "\\(%d+)([ac]^*[^\\]*)",
							function( numbers, values )
								if numbers:len( ) > 1 then
									local coloralpha = ""
									for i = 1, numbers:len( ) do
										coloralpha = coloralpha .. "\\" .. numbers:sub( i, i ) .. values
									end
									return coloralpha
								end
							end
						) --"\\134amrR( 100, 200 )"
						local colors = {
							[ 1 ] = "\\(%d%d+)c(%b())([%-%~]*)",
							[ 2 ] = "\\(%d%d+)c(&H%x+&)([%-%~]*)",
							[ 3 ] = "\\(%d%d+)c(R%b())([%-%~]*)",
							------------------------------------------
							[ 4 ] = "\\(%d%d+)c(%b())([%-%~]^*%d+[%.%d]*)",
							[ 5 ] = "\\(%d%d+)c(&H%x+&)([%-%~]^*%d+[%.%d]*)",
							[ 6 ] = "\\(%d%d+)c(R%b())([%-%~]^*%d+[%.%d]*)",
							------------------------------------------
							[ 7 ] = "\\(%d%d+)c(%b())([%-%~]^*%b())",
							[ 8 ] = "\\(%d%d+)c(&H%x+&)([%-%~]^*%b())",
							[ 9 ] = "\\(%d%d+)c(R%b())([%-%~]^*%b())",
						}
						for i = 1, #colors do
							String = String:gsub( colors[ i ],
								function( nums, val, sign_val )
									local str_col = ""
									local tbl_num = ke4.table.string( tostring( nums ) )
									for k = 1, #tbl_num do
										str_col = str_col .. format( "\\%sc%s", tbl_num[ k ], val .. sign_val )
									end
									return "@" .. str_col .. "@"
								end		
							)
						end
						return String
					end --"\\34cR( )"
					--▼--------------------------------------------------------
					String = multi_colors( String )
					String = multi_alphas( String )
					--▼--------------------------------------------------------
					String = String:gsub( "(\\[%dv]*c)(%([ ]*R%b()[ ]*%))",
						function( Tag, Random_funct )
							local Random_funct = Random_funct:match( "R%b()" ):sub( 2, -1 )
							return format( "%s( ke4.random.color2%s )", Tag, Random_funct )
						end
					) --"\\1cR( ** )" --> "\\1c .. ke4.random.color2( ** )"
					--▼--------------------------------------------------------
					String = String:gsub( "\\bs(%d[fr%.%d]*)", "\\bord%1\\shad%1" )
					--▼--------------------------------------------------------
					local tag_fun = {
						-- tags funciones: todos aquellos que están seguidos de paréntesis
						[ 001 ] = "\\1vc",		[ 002 ] = "\\2vc",		[ 003 ] = "\\3vc",		[ 004 ] = "\\4vc",
						[ 005 ] = "\\1va",		[ 006 ] = "\\2va",		[ 007 ] = "\\3va",		[ 008 ] = "\\4va",
						[ 009 ] = "\\1img",		[ 010 ] = "\\2img",		[ 011 ] = "\\3img",		[ 012 ] = "\\4img",
						[ 013 ] = "\\pos",		[ 014 ] = "\\move",		[ 015 ] = "\\moves3",	[ 016 ] = "\\moves4",
						[ 017 ] = "\\mover",	[ 018 ] = "\\movevc",	[ 019 ] = "\\org",		[ 020 ] = "\\distort",
						[ 021 ] = "\\fad",		[ 022 ] = "\\fade",		[ 023 ] = "\\jitter",	[ 024 ] = "\\clip",
						[ 025 ] = "\\iclip",	[ 026 ] = "\\t",
					}
					local tags_out = {
						-- los tags que quedan excluidos de ser remplazados en esta función, ya que de otro
						-- modo,  el valor random sería el mismo para cada uno de los tags que los componen
						[ 001 ] = "\\fscxy(R",	[ 002 ] = "\\frxy(R",	[ 003 ] = "\\frxz(R",	[ 004 ] = "\\fryz(R",
						[ 005 ] = "\\frxyz(R",	[ 006 ] = "\\faxy(R",	[ 007 ] = "\\fscxyr(R",	[ 008 ] = "\\frxyo(R",
						[ 009 ] = "\\frxzo(R",	[ 010 ] = "\\fryzo(R",	[ 011 ] = "\\frxyzo(R",	[ 012 ] = "\\xybord(R",
						[ 013 ] = "\\xyshad(R",
					}
					--▼--------------------------------------------------------
					String = String:gsub( "\\%w+%b()", -- tags seguidos de paréntesis :)
						function( capture )
							-------------------------------------------------------------------
							local tag_capture = ke4.tag.operation( capture:match( "(\\%w+)%b()" ) )
							local val_capture = capture:match( "\\%w+(%b())" ):sub( 2, -2 )
							-------------------------------------------------------------------
							if ke4.table.inside( tags_out, capture:match( "\\%w+%(R" ) ) == false then
								if type( ke4.table.index( tag_fun, tag_capture ) ) == "number" then
									--[ 1 ] tag \\t
									if ke4.table.index( tag_fun, tag_capture ) == 26 then
										val_capture = val_capture:gsub( "\\%w+%b()",
											function( time_capture )
												--------------------------------------------------------------------------
												local t_tag_capture = ke4.tag.operation( time_capture:match( "(\\%w+)%b()" ) )
												local t_val_capture = time_capture:match( "\\%w+(%b())" ):sub( 2, -2 )
												--------------------------------------------------------------------------
												if ke4.table.inside( tags_out, time_capture:match( "\\%w+%(R" ) ) == false then
													--[ 2 ]: tags funciones dentro de una transformación
													if type( ke4.table.index( tag_fun, t_tag_capture ) ) == "number" then
														if ke4.table.index( tag_fun, t_tag_capture ) >= 13
															and ke4.table.index( tag_fun, t_tag_capture ) <= 25 then
															if type( ke4.string.toval( "{" .. t_val_capture .. "}" ) ) == "table" then
																return format( "%s(%s)",
																	t_tag_capture,
																	ke4.table.op( ke4.string.toval( "{" .. t_val_capture .. "}" ), "concat", "," )
																)
															end --"\\t(\\clip( line.left, line.top, line.right, line.bottom ))"
															return format( "%s(%s)", t_tag_capture, t_val_capture )
														end
													end
													--[ 3 ]: tags funciones del 1 al 12 dentro de una transformación
													if ke4.table.inside( tag_fun, t_tag_capture ) then -- tags del MOD
														return time_capture
													end
													--[ 4 ]: tags dentro de una transformación
													return t_tag_capture .. ke4.string.toval( t_val_capture )
													--"\\t(\\blur( 3 + demo ))"
												end
												return time_capture
											end
										)
										return format( "%s(%s)", tag_capture, val_capture )
									end --"\\t(0,1000,\\blurR( 8 ))"
									--[ 5 ]: tags de posición, \\org y otros tags funciones
									if ke4.table.index( tag_fun, tag_capture ) >= 13 then
										if type( ke4.string.toval( "{" .. val_capture .. "}" ) ) == "table" then
											return format( "%s(%s)", tag_capture, ke4.table.op( ke4.string.toval( "{" .. val_capture .. "}" ), "concat", "," ) )
										end --"\\org( line.left, line.middle )"
										return capture
									end
									return capture
								end
								--[ 6 ]: tags que no son funciones
								return tag_capture .. ke4.string.toval( val_capture )
								--"\\blur( 3 + 5i )"
							end
							return capture
						end
					)
					--▼--------------------------------------------------------
					return String
				end
				------------------------------------------------------------------------------------------
				String = redefinex( String )
				------------------------------------------------------------------------------------------
				local tags_i = {
					[ 01 ] = "(\\fscxy)",			[ 02 ] = "(\\frxy)",		[ 03 ] = "(\\frxz)",
					[ 04 ] = "(\\fryz)",			[ 05 ] = "(\\frxyz)",		[ 06 ] = "(\\faxy)",
					[ 07 ] = "(\\fscxr)",			[ 08 ] = "(\\fscyr)",		[ 09 ] = "(\\fscxyi)",
					[ 10 ] = "(\\fscxy[i]*r)",		[ 11 ] = "(\\xybord)",		[ 12 ] = "(\\xyshad)",
				}
				------------------------------------------------------------------------------------------
				local tags_r = {
					[ 01 ] = "\\fscx%s\\fscy%s",			[ 02 ] = "\\frx%s\\fry%s",
					[ 03 ] = "\\frx%s\\frz%s",				[ 04 ] = "\\fry%s\\frz%s",
					[ 05 ] = "\\frx%s\\fry%s\\frz%s",		[ 06 ] = "\\fax%s\\fay%s",
					[ 07 ] = "\\fscxr%s",					[ 08 ] = "\\fscyr%s",
					[ 09 ] = "\\fscx%s\\fscy%s",			[ 10 ] = "\\fscxr%s\\fscyr%s",
					[ 11 ] = "\\xbord%s\\ybord%s",			[ 12 ] = "\\xshad%s\\yshad%s",
				}
				------------------------------------------------------------------------------------------
				local tags_v = {
					[ 01 ] = "(%-?%d+[%.%d]*)",
					[ 02 ] = "(R[cdemrs]*%b())",
					[ 03 ] = "(%b())",
				}
				------------------------------------------------------------------------------------------
				String = String:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
				------------------------------------------------------------------------------------------
				for i = 1, #tags_i do
					for k = 1, 3 do
						String = String:gsub( tags_i[ i ] .. tags_v[ k ],
							function( tagdark, capture )
								if pcall( loadstring( format( "return function( meta, line, x, y ) return %s end", capture ) ) ) then
									local fun_dark = loadstring( format( "return function( meta, line, x, y ) return %s end", capture ) )( )
									if pcall( fun_dark ) then
										if tags_i[ i ]:match( "\\fscxyi" )
											and fun_dark( meta, line, x, y ) then
											local size_xy = fun_dark( meta, line, x, y )
											if size_xy then
												return format( tags_r[ i ], size_xy, size_xy )
											end
										end --"\\fscxyiRm( 100, 200 )"
										if fun_dark( meta, line, x, y ) then
											return format( tags_r[ i ],
												fun_dark( meta, line, x, y ),
												fun_dark( meta, line, x, y ),
												fun_dark( meta, line, x, y )
											) --"\\frxyzRds( 360 )"
										end
									end
								end
								return tagdark .. capture
							end
						)
					end
				end
				--------------------------------------------------------------------------
				String = String:gsub( "\"(&H%x+&)\"", "%1" )
				String = String:gsub( "(\\%w+%-?[%d&#]^*[%.%dH&%x]*) [	 ]*(\\)", "%1%2" )
				--------------------------------------------------------------------------
				-- extrae el tag \\org que esté dentro de una transformación
				String = String:gsub( "\\t%b()",
					function( time_cap )
						if time_cap:match( "\\org%b()" ) then
							local org_tag = time_cap:match( "\\org%b()" )
							time_cap = org_tag .. time_cap:gsub( "\\org%b()", "" )
						end
						return time_cap
					end
				)
				------------------------------------------------
				local function do_alphax( String )
					local tag_alpha = {
						[ 1 ] = "\\alpha(%d+[%.%d]*)",
						[ 2 ] = "\\1a(%d+[%.%d]*)",
						[ 3 ] = "\\2a(%d+[%.%d]*)",
						[ 4 ] = "\\3a(%d+[%.%d]*)",
						[ 5 ] = "\\4a(%d+[%.%d]*)",
						[ 6 ] = "\\1va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
						[ 7 ] = "\\2va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
						[ 8 ] = "\\3va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
						[ 9 ] = "\\4va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
					}
					local tag_replace = {
						[ 1 ] = "\\alpha%s",
						[ 2 ] = "\\1a%s",
						[ 3 ] = "\\2a%s",
						[ 4 ] = "\\3a%s",
						[ 5 ] = "\\4a%s",
						[ 6 ] = "\\1va(%s,%s,%s,%s)",
						[ 7 ] = "\\2va(%s,%s,%s,%s)",
						[ 8 ] = "\\3va(%s,%s,%s,%s)",
						[ 9 ] = "\\4va(%s,%s,%s,%s)",
					}
					for i = 1, 5 do
						String = String:gsub( tag_alpha[ i ],
							function( num )
								return format( tag_replace[ i ], ke4.alpha.val2ass( num ) )
							end
						)
					end
					for i = 6, 9 do
						String = String:gsub( tag_alpha[ i ],
							function( num1, num2, num3, num4 )
								return format( tag_replace[ i ],
									ke4.alpha.val2ass( num1 ), ke4.alpha.val2ass( num2 ),
									ke4.alpha.val2ass( num3 ), ke4.alpha.val2ass( num4 )
								)
							end
						)
					end
					return String
				end
				String = redefinex( String )
				String = do_alphax( String )
				------------------------------------------------
				String = String:gsub( "'(&H%x+&)'", "%1" )
				------------------------------------------------
				local function tag_round( TagString )
					-- redondea los valores numéricos de los tags
					local dec_round = 3
					TagString = TagString:gsub( "(%-?%d+%.%d+)",
						function( number )
							return ke4.math.round( tonumber( number ), dec_round )
						end
					)
					return TagString
				end
				------------------------------------------------
				return tag_round( ke4.string.change( String, "\\org%b()", 1 ) )
			end,
			
			set = function( times_set, events_set, Line_start_time, Line_end_time )
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				if type( times_set ) == "table" then
					times_set = times_set
				else
					times_set = { times_set }
				end
				if type( events_set ) == "table" then
					events_set = events_set
				else
					events_set = { events_set }
				end
				local t_set = ke4.table.complete( times_set, Line_start_time, Line_end_time )
				local t_st2 = ke4.table.duplicate( t_set )
				local iSt, iEt = ke4.table.index( t_set, Line_start_time ), ke4.table.index( t_set, Line_end_time )
				events_set[ 0 ] = ""
				local i = iSt + 1
				local Tag_fx = events_set[ i - 2 ]
				for k = 1, #t_set do
					if type( t_set[ k ] ) == "table" then
						t_set[ k ] = t_set[ k ][ 1 ]
					end
				end
				if iSt + 1 ~= iEt then
					while t_set[ i ] < Line_end_time do
						offset_t = 1
						if type( t_st2[ i ] ) == "table" then
							offset_t = t_st2[ i ][ 2 ]
						end
						t1 = t_set[ i ] - Line_start_time - frame_dur / 2
						if t1 < 0 then
							t1 = 0
						end
						t2 = t1 + offset_t
						Tag_fx = Tag_fx .. format( "\\t(%s,%s,%s)", t1, t2, events_set[ i - 1 ] )
						i = i + 1
					end
				end
				return Tag_fx
			end,

			only = function( condition, s_true, s_false )
				if condition then
					return s_true
				end
				return s_false or ( (type( s_true ) == "number") and 0 or "" )
			end,

			only2 = function( Conditions, ... )
				local TrueExits = { ... }
				if type( ... ) == "table" then
					TrueExits = ...
				end
				for i = 1, #Conditions do
					if Conditions[ i ] then
						return TrueExits[ i ]
					end
				end
				return ( type( TrueExits[ #TrueExits ] ) == "number" ) and 0 or ""
			end,

			movevc = function( Shape, posx, posy, Dx, Dy, Time_i, Time_f )
				local ti, tf = Time_i, Time_f
				local DX, DY = Dx, Dy
				local PX, PY = posx, posy
				local Shape = ke4.shape.origin( ke4.shape.ASSDraw3( Shape ) )
				ke4.shape.info( Shape )
				local Tag_fx = format( "\\clip(%s)\\movevc(%s,%s,%s,%s,%s,%s)", Shape,
					ke4.math.round( PX - w_shape / 2, 3 ),		ke4.math.round( PY - h_shape / 2, 3 ),
					ke4.math.round( PX - w_shape / 2 + DX, 3 ),	ke4.math.round( PY - h_shape / 2 + DY, 3 ),
					ke4.math.round( ti, 3 ),					ke4.math.round( tf, 3 )
				)
				return Tag_fx
			end,

			movevci = function( Shape, posx, posy, Dx, Dy, Time_i, Time_f )
				local  Tag_movevci = ke4.tag.movevc( Shape, posx, posy, Dx, Dy, Time_i, Time_f ):gsub( "clip", "iclip" )
				return Tag_movevci
			end,

			glitter = function( DurTotal, ExtraTags_i, ExtraTags_f )
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local time_ini, t, t1, t2, t3, t4, Tag_fx = 0, 0, 0, 0, 0, 0, "\\shad0"
				local GT_i = ExtraTags_i or ""
				local GT_f = ExtraTags_f or ""
				local time_fin = DurTotal
				if type( DurTotal ) == "table" then
					time_ini = DurTotal[ 1 ] or 0
					time_fin = DurTotal[ 2 ] or 5000
				elseif type( DurTotal ) == "function" then
					time_ini = DurTotal( )[ 1 ] or 0
					time_fin = DurTotal( )[ 2 ] or 5000
				end
				local time_tot = time_fin - time_ini
				local i = 0
				while time_tot > t do
					t1 = t + ke4.math.Rc( 1, time_tot / 2 )
					if t1 >= time_tot then
						break
					end
					t2 = t1 + 1
					t3 = t2 + frame_dur
					t4 = t3 + 2.5 * frame_dur
					size1 = ke4.math.Rc( 1.5 * 100, 2.5 * 100 )
					size2 = ke4.math.Rc( 0.5 * 100, 1.5 * 100 )
					if t4 > time_tot then
						t4 = time_tot
					end
					Tag_fx = Tag_fx .. format( "\\t(%d,%d,%s\\fscx%d\\fscy%d)\\t(%d,%d,%s\\fscx%d\\fscy%d)",
						time_ini + t1, time_ini + t2, GT_i, size1, size1,
						time_ini + t3, time_ini + t4, GT_f, size2, size2
					)
					t = t4
					i = i + 1
				end
				Tag_fx = ke4.string.i( Tag_fx )
				return Tag_fx 
			end, --!_G.ke4.tag.glitter( line.duration )!
			
			glitterx = function( DurTotal, ExtraTags_i, ExtraTags_f )
				local  tag_glitterx = ke4.tag.glitter( DurTotal, ExtraTags_i, ExtraTags_f ):gsub( "\\fscy%d+[%.%d]*", "" )
				return tag_glitterx
			end, --!_G.ke4.tag.glitterx( line.duration )!
			
			glittery = function( DurTotal, ExtraTags_i, ExtraTags_f )
				local  tag_glittery = ke4.tag.glitter( DurTotal, ExtraTags_i, ExtraTags_f ):gsub( "\\fscx%d+[%.%d]*", "" )
				return tag_glittery
			end, --!_G.ke4.tag.glittery( line.duration )!
			
			clip_shape = function( Shapes, Center_x, Center_y )
				local Center_x = Center_x or 0
				local Center_y = Center_y or 0
				local Shp_tbl = ke4.shape.divide( ke4.shape.centerpos( Shapes, Center_x, Center_y ) )
				return Shp_tbl
			end, --{\clip(!_G.ke4.tag.clip_shape( _G.ke4.shape.circle, $x, $y )[ 1 ]!)}

			ipol = function( Ipol_i, ... )
				local valors = { ... }
				if type( ... ) == "table" then
					valors = ...
				end
				---------------------------------------------
				-- interpola el valor de dos números
				local function ipol_number( val_1, val_2, pct_ipol )
					if type( val_1 ) == "number" and type( val_2 ) == "number" then
						return val_1 + (val_2 - val_1) * pct_ipol
					end
					return val_2
				end
				---------------------------------------------
				-- interpola el valor de dos shapes o dos clips
				local function ipol_shpclip( val_1, val_2, pct_ipol )
					local function ipairx( Shape1, Shape2 )
						local function parts( Shape )
							--partes que posee una shape
							local Parts = { }
							for p in Shape:gmatch( "[mlb]^*%s+%-?%d+[%.%d]*%s+[%-%.%d ]*" ) do
								table.insert( Parts, p )
							end
							return Parts
						end
						local function count( Shape )
							--cantidad de puntos que posee una shape
							local n = 0
							local Shape = Shape:gsub( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*",
								function( point )
									n = n + 1
								end
							)
							return n
						end
						local Parts1 = parts( Shape1 )
						local Parts2 = parts( Shape2 )
						local Point1 = count( Shape1 )
						local Point2 = count( Shape2 )
						if Point1 >= Point2 then
							return Shape1
						end
						local difere = Point2 - Point1
						local addpoi = ceil( difere / 3 )
						local idx = ( floor( #Parts1 / addpoi ) > 0 ) and floor( #Parts1 / addpoi ) or 1
						local xpoint
						for i = 1, addpoi do
							xpoint = Parts1[ (idx * i - 1) % #Parts1 + 1 ]:match( "%-?%d+[%.%d]*%s+%-?%d+[%.%d]*" ) .. " "
							Parts1[ (idx * i - 1) % #Parts1 + 1 ] = Parts1[ (idx * i - 1) % #Parts1 + 1 ] .. "b " .. xpoint .. xpoint .. xpoint
						end
						return table.concat( Parts1 )
					end
					-----------------------------------
					local val_1 = ipairx( val_1, val_2 )
					local tbl_1, tbl_2, k = { }, { }, 1
					for px, py in val_1:gmatch( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)" ) do
						tbl_1[ #tbl_1 + 1 ] = { tonumber( px ), tonumber( py ) }
					end
					for px, py in val_2:gmatch( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)" ) do
						tbl_2[ #tbl_2 + 1 ] = { tonumber( px ), tonumber( py ) }
					end
					--------------
					local function ipair_fx( num_min, num_max )
						--genera una tabla de "emparejamiento"
						local n_min = math.min( num_min, num_max )
						local n_max = math.max( num_min, num_max )
						local inter = floor( n_max / n_min )
						local modul = n_max % n_min
						local index = { }
						for i = 1, inter * n_min do
							index[ i ] = ke4.math.i( i, inter )[ "N,n" ]
						end
						for i = 1, modul do
							table.insert( index, floor( n_max / modul ) * i, index[ floor( n_max / modul ) * i - 1 ] )
						end
						return index
					end
					--------------
					local tbl_ipar = ipair_fx( #tbl_1, #tbl_2 )
					local val_ipol, idx
					val_ipol = val_1:gsub( "(%-?%d+[%.%d]*)%s+(%-?%d+[%.%d]*)",
						function( val_x, val_y )
							idx = tbl_ipar[ k ]
							local val_x = tbl_1[ k ][ 1 ] + ( tbl_2[ idx ][ 1 ] - tbl_1[ k ][ 1 ]) * pct_ipol
							local val_y = tbl_1[ k ][ 2 ] + ( tbl_2[ idx ][ 2 ] - tbl_1[ k ][ 2 ]) * pct_ipol
							k = k + 1
							return ke4.math.round( val_x, 3 ) .. " " .. ke4.math.round( val_y, 3 )
						end
					)
					return val_ipol
				end
				---------------------------------------------
				-- busca un string dentro de la tabla
				local function string_in_tbl( str_in_tbl )
					for i = 1, #str_in_tbl do
						if type( str_in_tbl[ i ] ) == "string" then
							return true, str_in_tbl[ i ]
						end
					end
					return false
				end
				---------------------------------------------
				-- determina si los elementos son clip's o shapes
				local function type_table( Table )
					if type( Table[ 1 ] ) == "string" then
						if Table[ 1 ]:match( "%([ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%)" ) then
							for i = 1, #Table do
								if type( Table[ i ] ) ~= "string"
									or not Table[ i ]:match( "%([ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*%)" ) then
									return nil
								end
							end
							return true
						elseif Table[ 1 ]:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
							for i = 1, #Table do
								if type( Table[ i ] ) ~= "string"
									or not Table[ i ]:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" ) then
									return nil
								end
							end
							return true
						end
					end
					return nil
				end
				-------------------------------------------------------------
				-- decide cuál de las 4 interpolaciones se usará
				local ipol_function = ipol_number
				local Shp_or_Clip = type_table( valors )
				if Shp_or_Clip then
					ipol_function = ipol_shpclip
				end
				if string_in_tbl( valors ) then
					local True, Val_str = string_in_tbl( valors )
					if Val_str:match( "[&Hh%#]^*%x%x%x%x%x%x[&]*" ) then
						ipol_function = ke4.color.interpolate
					elseif Val_str:match( "[&Hh%#]^*%x%x[&]*" ) then
						ipol_function = ke4.alpha.interpolate
					end
				end
				---------------------------------------------
				local Ipol_i = Ipol_i or 0.5
				Ipol_i = ke4.math.clamp( Ipol_i ) * (#valors - 1)
				local valor_i = valors[ floor( Ipol_i ) + 1 ]
				local valor_f = valors[ floor( Ipol_i ) + 2 ] or valors[ floor( Ipol_i ) + 1 ]
				return ipol_function( valor_i, valor_f, Ipol_i - floor( Ipol_i ) )
			end,
			
			clip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
				local loop_W, loop_H = loop_x, loop_y
				local offset = 3--l.outline + l.shadow
				local left_x, top_y = Left - offset, Top - offset
				local clip_W, clip_H = Width + 2 * offset, Height + 2 * offset
				local mode = Mode or 79
				local size_W, size_H = clip_W / loop_W, clip_H / loop_H
				local cx1, cx2, cy1, cy2
				
				if ke4.table.inside( { 13, 17, 31, 39, 71, 79, 93, 97 }, mode ) == false then
					mode = 79
				end
				if mode == 17
					or mode == 71 then
					cx1 = ke4.math.round( left_x + (ceil( j / loop_H ) - 1) * size_W, 3 )
					cx2 = ke4.math.round( left_x + (ceil( j / loop_H ) - 0) * size_W, 3 )
				elseif mode == 13
					or mode == 79 then
					cx1 = ke4.math.round( left_x + ((j - 1) % loop_W + 0) * size_W, 3 )
					cx2 = ke4.math.round( left_x + ((j - 1) % loop_W + 1) * size_W, 3 )
				elseif mode == 39
					or mode == 93 then
					cx1 = ke4.math.round( left_x + clip_W - (ceil( j / loop_H ) - 0) * size_W, 3 )
					cx2 = ke4.math.round( left_x + clip_W - (ceil( j / loop_H ) - 1) * size_W, 3 )
				elseif mode == 31
					or mode == 97 then
					cx1 = ke4.math.round( left_x + clip_W - ((j - 1) % loop_W + 1) * size_W, 3 )
					cx2 = ke4.math.round( left_x + clip_W - ((j - 1) % loop_W + 0) * size_W, 3 )
				end
				if mode == 79 or
					mode == 97 then
					cy1 = ke4.math.round( top_y + (ceil( j / loop_W ) - 1) * size_H, 3 )
					cy2 = ke4.math.round( top_y + (ceil( j / loop_W ) - 0) * size_H, 3 )
				elseif mode == 71
					or mode == 93 then
					cy1 = ke4.math.round( top_y + ((j - 1) % loop_H + 0) * size_H, 3 )
					cy2 = ke4.math.round( top_y + ((j - 1) % loop_H + 1) * size_H, 3 )
				elseif mode == 13
					or mode == 31 then
					cy1 = ke4.math.round( top_y + clip_H - (ceil( j / loop_W ) - 0) * size_H, 3 )
					cy2 = ke4.math.round( top_y + clip_H - (ceil( j / loop_W ) - 1) * size_H, 3 )
				elseif mode == 17
					or mode == 39 then
					cy1 = ke4.math.round( top_y + clip_H - ((j - 1) % loop_H + 1) * size_H, 3 )
					cy2 = ke4.math.round( top_y + clip_H - ((j - 1) % loop_H + 0) * size_H, 3 )
				end
				return format( "\\clip(%s,%s,%s,%s)", cx1, cy1, cx2, cy2 )
				--code once: loop_x = 5 loop_y = 3
			end, --!maxloop( loop_x * loop_y )!{\an5\pos($x,$y)!_G.ke4.tag.clip( j, loop_x, loop_y, line.left, line.top, line.width, line.height, nil )!}
			
			iclip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
				local iclip_tag = ke4.tag.clip( j, loop_x, loop_y, Left, Top, Width, Height, Mode ):gsub( "clip", "iclip" )
				return iclip_tag
			end,

			clip2 = function( Left, Top, Width, Height )
				local offset = 3--l.outline + l.shadow
				local left_x = Left - offset
				local top_y  = Top - offset
				local clip_W = Width + 2 * offset
				local clip_H = Height + 2 * offset
				local cx1, cx2 = ke4.math.round( left_x, 3 ), ke4.math.round( left_x + clip_W, 3 )
				local cy1, cy2 = ke4.math.round( top_y, 3 ), ke4.math.round( top_y + clip_H, 3 )
				return format( "\\clip(%s,%s,%s,%s)", cx1, cy1, cx2, cy2 )
			end, --{\an5\pos($x,$y)!_G.ke4.tag.clip2( line.left + syl.left, line.top, syl.width, syl.height )!}

			iclip2 = function( Left, Top, Width, Height )
				local iclip2_tag = ke4.tag.clip2( Left, Top, Width, Height ):gsub( "clip", "iclip" )
				return iclip2_tag
			end,
			
			rclip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
				local offset = 3--l.outline + l.shadow
				local mode = Mode or 79
				local left_x = Left - offset
				local top_y = Top - offset
				local clip_W, clip_H = Width + 2 * offset, Height + 2 * offset
				local loop_W, loop_H = loop_x, loop_y
				local pixelW, pixelH = recall.pixx, recall.pixy
				if j == 1 then
					pixelW = remember( "pixx", { } )
					pixelH = remember( "pixy", { } )
					pixelW[ -1 ] = 0
					pixelH[ -1 ] = 0
					for i = 0, loop_W - 1 do
						pixelW[ i ] = pixelW[ i - 1 ] + clip_W / loop_W + ke4.math.R( -clip_W / (2 * loop_W), clip_W / (2 * loop_W) )
					end
					for i = 0, loop_H - 1 do
						pixelH[ i ] = pixelH[ i - 1 ] + clip_H / loop_H + ke4.math.R( -clip_H / (2 * loop_H), clip_H / (2 * loop_H) )
					end
					pixelW[ loop_W - 1 ] = clip_W
					pixelH[ loop_H - 1 ] = clip_H
				end
				local cx1, cx2, cy1, cy2
				if ke4.table.inside( { 13, 17, 31, 39, 71, 79, 93, 97 }, mode ) == false then
					mode = 79
				end
				if mode == 17
					or mode == 71 then
					cx1 = left_x + pixelW[ ceil( j / loop_H ) - 2 ]
					cx2 = left_x + pixelW[ ceil( j / loop_H ) - 1 ]
				elseif mode == 13
					or mode == 79 then
					cx1 = left_x + pixelW[ (j - 1) % loop_W - 1 ]
					cx2 = left_x + pixelW[ (j - 1) % loop_W - 0 ]
				elseif mode == 39
					or mode == 93 then
					cx1 = left_x + clip_W - pixelW[ ceil( j / loop_H ) - 1 ]
					cx2 = left_x + clip_W - pixelW[ ceil( j / loop_H ) - 2 ]
				elseif mode == 31
					or mode == 97 then
					cx1 = left_x + clip_W - pixelW[ (j - 1) % loop_W - 0 ]
					cx2 = left_x + clip_W - pixelW[ (j - 1) % loop_W - 1 ]
				end
				if mode == 79
					or mode == 97 then
					cy1 = top_y + pixelH[ ceil( j / loop_W ) - 2 ]
					cy2 = top_y + pixelH[ ceil( j / loop_W ) - 1 ]
				elseif mode == 71
					or mode == 93 then
					cy1 = top_y + pixelH[ (j - 1) % loop_H - 1 ]
					cy2 = top_y + pixelH[ (j - 1) % loop_H - 0 ]
				elseif mode == 13
					or mode == 31 then
					cy1 = top_y + clip_H - pixelH[ ceil( j / loop_W ) - 1 ]
					cy2 = top_y + clip_H - pixelH[ ceil( j / loop_W ) - 2 ]
				elseif mode == 17
					or mode == 39 then
					cy1 = top_y + clip_H - pixelH[ (j - 1) % loop_H - 0 ]
					cy2 = top_y + clip_H - pixelH[ (j - 1) % loop_H - 1 ]
				end
				return format( "\\clip(%s,%s,%s,%s)",
					ke4.math.round( cx1, 2 ), ke4.math.round( cy1, 2 ), ke4.math.round( cx2, 2 ), ke4.math.round( cy2, 2 )
				)
				--code once: loop_x = 5 loop_y = 3
			end, --!maxloop( loop_x * loop_y )!{\an5\pos($x,$y)!_G.ke4.tag.rclip( j, loop_x, loop_y, line.left, line.top, line.width, line.height )!}

			riclip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode )
				local riclip_tag = ke4.tag.rclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode ):gsub( "clip", "iclip" )
				return riclip_tag
			end,
			
			moveclip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
				local left_x, top_y = Left, Top
				local Move_x = Move_x or 0
				local Move_y = Move_y or 0
				local clip_i = ke4.tag.clip( j, loop_x, loop_y, left_x, top_y, Width, Height, Mode )
				local clip_f = ke4.tag.clip( j, loop_x, loop_y, left_x + Move_x, top_y + Move_y, Width, Height, Mode )
				if Time_i == nil
					or Time_f == nil then
					return format( "%s\\t(%s)", clip_i, clip_f )
				end
				return format( "%s\\t(%s,%s,%s)", clip_i, Time_i, Time_f, clip_f )
				--code once: loop_x = 5 loop_y = 3
			end, --!maxloop( loop_x * loop_y )!{\an5\pos($x,$y)!_G.ke4.tag.moveclip( j, loop_x, loop_y, line.left + syl.left, line.top, syl.width, syl.height, nil, 200, 200 )!}
			
			moveiclip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
				local moveiclip_tag = ke4.tag.moveclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f ):gsub( "clip", "iclip" )
				return moveiclip_tag
			end,
			
			moveclip2 = function( Left, Top, Width, Height, Move_x, Move_y, Time_i, Time_f )
				local Move_x = Move_x or 0
				local Move_y = Move_y or 0
				local moveclip2_i = ke4.tag.clip2( Left, Top, Width, Height )
				local moveclip2_f = ke4.tag.clip2( Left + Move_x, Top + Move_y, Width, Height )
				if Time_i == nil
					or Time_f == nil then
					return format( "%s\\t(%s)", moveclip2_i, moveclip2_f )
				end
				return format( "%s\\t(%s,%s,%s)", moveclip2_i, Time_i, Time_f, moveclip2_f )
			end, --{\an5\pos($x,$y)!_G.ke4.tag.moveclip2( line.left + syl.left, line.top, syl.width, syl.height, 200, 200 )!}

			moveiclip2 = function( Left, Top, Width, Height, Move_x, Move_y, Time_i, Time_f )
				local moveiclip2_tag = ke4.tag.moveclip2( Left, Top, Width, Height, Move_x, Move_y, Time_i, Time_f ):gsub( "clip", "iclip" )
				return moveiclip2_tag
			end,
			
			moverclip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
				local left_x, top_y = Left, Top
				local Move_x = Move_x or 0
				local Move_y = Move_y or 0
				local clip_i = ke4.tag.rclip( j, loop_x, loop_y, left_x, top_y, Width, Height, Mode )
				local clip_f = ke4.tag.rclip( j, loop_x, loop_y, left_x + Move_x, top_y + Move_y, Width, Height, Mode )
				if Time_i == nil
					or Time_f == nil then
					return format( "%s\\t(%s)", clip_i, clip_f )
				end
				return format( "%s\\t(%s,%s,%s)", clip_i, Time_i, Time_f, clip_f )
			end,
			
			movericlip = function( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f )
				local movericlip_tag = ke4.tag.moverclip( j, loop_x, loop_y, Left, Top, Width, Height, Mode, Move_x, Move_y, Time_i, Time_f ):gsub( "clip", "iclip" )
				return movericlip_tag
			end,

			operation = function( String )
				local String = String
				:gsub( "%%","%%%%" ):gsub( "%*", "%%*" ):gsub( "%+", "%%+" ):gsub( "%-", "%%-" )
				:gsub( "%(", "%%(" ):gsub( "%)", "%%)" ):gsub( "%[", "%%[" ):gsub( "%]", "%%]" )
				return String
			end,
			
			lmove = function( Configs, Coor, Times, Times2 ) -- lineal move
				local l_descent = Configs[ 1 ].descent
				local val_width, val_height = Configs[ 2 ], Configs[ 3 ]
				if type( Times ) == "number" then
					local time_ini, time_dur = 0, Times
					local time1, time2 = 0, 0
					if Times2 then
						--asigna un único tiempo de incio y uno final
						if type( Times2 ) == "number" then
							time_dur = Times2
						elseif type( Times2 ) == "table" then
							time_ini = Times2[ 1 ] or 0
							time_dur = (Times2[ 2 ] or Times) - time_ini
						end
					end
					local segments = { }
					for i = 1, #Coor / 2 - 1 do
						segments[ i ] = ke4.math.distance( Coor[ 2 * i - 1 ], Coor[ 2 * i ], Coor[ 2 * i + 1 ], Coor[ 2 * i + 2 ] )
					end
					local total_length = ke4.table.op( segments, "sum" )
					local parcial = { [ 0 ] = 0 }
					for i = 1, #segments do
						parcial[ i ] = segments[ i ] + parcial[ i - 1 ]
					end
					local Times = { [ 0 ] = time_ini }
					for i = 1, #Coor - 2 do
						Times[ i ] = (i % 2 == 1)
						and Times[ i - 1 ]
						or ke.math.round( time_ini + time_dur * parcial[ i / 2 ] / total_length, 2 )
					end
				end
				local coorx, coory = { }, { }
				for i = 1, #Coor / 2 do
					coorx[ i ] = ke4.math.round( Coor[ 2 * i - 1 ] - val_width / 2, 2 )
					coory[ i ] = ke4.math.round( Coor[ 2 * i - 0 ] + val_height / 2 - l_descent, 2 )
				end
				local tags1 = format( "\\fscx%s\\fscy%s", coorx[ 1 ], coory[ 1 ] )
				for i = 2, #coorx do
					tags1 = tags1 .. format( "\\t(%s,%s,\\fscx%s\\fscy%s)",
						Times[ 2 * (i - 1) - 1 ], Times[ 2 * (i - 1) ], coorx[ i ], coory[ i ]
					)
				end
				local tags2 = format( "%s\\p1}m 0 0 m 100 100 {\\p0\\r}", tags1 )
				local fscxy, sizex, sizey = { }, { }, { }
				for c in tags2:gmatch( "fscx?y?(%-?%d+[%.%d]*)" ) do
					table.insert( fscxy, tonumber( c ) )
				end
				for i = 1, #fscxy / 2 do
					table.insert( sizex, fscxy[ 2 * i - 1 ] )
					table.insert( sizey, fscxy[ 2 * i - 0 ] )
				end
				local min_x, min_y = ke4.table.op( sizex, "min" ), ke4.table.op( sizey, "min" )
				if min_x >= 0 then
					min_x = 0
				end
				if min_y > 0
					and min_y <= val_height then
					min_y = -min_y
				elseif min_y >= 0 then
					min_y = 0
				end
				local posxy = format( "\\an7\\q2\\pos(%s,%s)", min_x, min_y )
				tags2 = "{".. posxy .. tags2:gsub( "(\\fscx%-?%d+[%.%d]*)(\\fscy%-?%d+[%.%d]*)", 
					function( x, y )
						fsc_x = x:match( "%-?%d+[%.%d]*" )
						fsc_y = y:match( "%-?%d+[%.%d]*" )
						return format( "\\fscx%s\\fscy%s", fsc_x - min_x, fsc_y - min_y )
					end
				)
				return tags2
				--code syl: Configs = { line, syl.width, syl.height }
			end, --!_G.ke4.tag.lmove( Configs, { $x - line.right, $y, $x, $y, $x + meta.res_x - line.left, $y }, { 0, 200, line.duration - 200, line.duration } )!
			
			pmove = function( Configs, F_x, F_y, domainF, t1, t2, Accel, offset_t ) -- Parametric Move
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local l_descent = Configs[ 1 ].descent
				local pos_x, pos_y = Configs[ 2 ], Configs[ 3 ]
				local val_width, val_height = Configs[ 4 ], Configs[ 5 ]
				local offset_t = offset_t or 0
				local accel = Accel or 1
				local time1 = t1 or 0
				local time2 = t2 or 5000--line.duration
				local tgdur
				local dur_t = time2 - time1
				if type( offset_t ) == "table" then
					tgdur = ke4.math.round( offset_t[ 1 ], 2)
				else
					tgdur = ke4.math.round( 2.4 * frame_dur + offset_t, 2 )
				end
				tgdur = abs( tgdur )
				local n, i, domain = ceil( dur_t / tgdur ), 0, { }
				if type( domainF ) == "number" then
					domain = { 0, domainF }
				else
					domain = { domainF[ 1 ], domainF[ 2 ] }
				end
				local posx = ke4.math.round( pos_x - val_width / 2 + ke4.math.format( F_x, domain[ 1 ] ), 3 )
				local posy = ke4.math.round( pos_y + val_height / 2 - l_descent - ke4.math.format( F_y, domain[ 1 ] ), 3 )
				local tags = format( "\\fscx%s\\fscy%s", posx, posy )
				while dur_t > 0 do
					if type( Accel ) == "function" then
						accel = Accel( )
					end
					tags = tags .. format( "\\t(%s,%s,1,\\fscx%s\\fscy%s)",
						time1 + tgdur * i, time1 + tgdur * (i + 1), 
						ke4.math.round( posx + ke4.math.format( F_x, domain[ 1 ] + (domain[ 2 ] - domain[ 1 ]) * ((i + 1) / n) ^ accel ) - ke4.math.format( F_x, domain[ 1 ] ), 3 ),
						ke4.math.round( posy - ke4.math.format( F_y, domain[ 1 ] + (domain[ 2 ] - domain[ 1 ]) * ((i + 1) / n) ^ accel ) + ke4.math.format( F_y, domain[ 1 ] ), 3 )
					)
					i = i + 1
					dur_t = dur_t - tgdur
				end
				local tags2 = format( "%s\\p1}m 0 0 m 100 100 {\\p0\\r}", tags )
				local fscxy, sizex, sizey = { }, { }, { }
				for c in tags2:gmatch( "fscx?y?(%-?%d+[%.%d]*)" ) do
					table.insert( fscxy, tonumber( c ) )
				end
				for i = 1, #fscxy / 2 do
					table.insert( sizex, fscxy[ 2 * i - 1 ] )
					table.insert( sizey, fscxy[ 2 * i - 0 ] )
				end
				local min_x, min_y = ke4.table.op( sizex, "min" ), ke4.table.op( sizey, "min" )
				if min_x >= 0 then
					min_x = 0
				end
				if min_y > 0
					and min_y <= val_height then
					min_y = -min_y
				elseif min_y >= 0 then
					min_y = 0
				end
				local posxy = format( "\\an7\\q2\\pos(%s,%s)", min_x, min_y )
				tags2 = "{" .. posxy .. tags2:gsub( "(\\fscx%-?%d+[%.%d]*)(\\fscy%-?%d+[%.%d]*)", 
					function( x, y )
						fsc_x = x:match( "%-?%d+[%.%d]*" )
						fsc_y = y:match( "%-?%d+[%.%d]*" )
						return format( "\\fscx%s\\fscy%s", fsc_x - min_x, fsc_y - min_y )
					end
				)
				tags2 = tags2:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" )
				return tags2
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.pmove( Configs, "100 * math.cos( %s )", "100 * math.sin( %s )", { 0, 2 * math.pi }, 0, line.duration )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!

			smove = function( Configs, Shape, t1, t2, Relative ) -- Shape Move
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				local l_descent = Configs[ 1 ].descent
				local pos_x, pos_y = Configs[ 2 ], Configs[ 3 ]
				local val_width, val_height = Configs[ 4 ], Configs[ 5 ]
				if Shape == nil
					and Configs[ 1 ].text:match( "\\i?clip%b()" ) then
					Shape = ke4.shape.ASSDraw3( Configs[ 1 ].text:match( "\\i?clip%b()" ) )
				end
				local time1 = t1 or 0
				local time2 = t2 or 5000--line.duration
				local dur_t, segms = time2 - time1, { }
				if type( Shape ) == "table" then
					if #Shape == 4 then
						Shape = format( "m %s %s l %s %s ", Shape[ 1 ], Shape[ 2 ], Shape[ 3 ], Shape[ 4 ] )
					elseif #Shape == 6 then
						Shape = format( "m %s %s b %s %s %s %s %s %s ", Shape[ 1 ], Shape[ 2 ], Shape[ 3 ], 
							Shape[ 4 ], Shape[ 3 ], Shape[ 4 ], Shape[ 5 ], Shape[ 6 ]
						)
					elseif #Shape == 8 then
						Shape = format( "m %s %s b %s %s %s %s %s %s ", Shape[ 1 ], Shape[ 2 ], Shape[ 3 ], 
							Shape[ 4 ], Shape[ 5 ], Shape[ 6 ], Shape[ 7 ], Shape[ 8 ]
						)
					end
				end
				local Shape = ke4.shape.ASSDraw3( Shape )
				local tags2 = ""--recall.shSmtag
				--if j == 1 then
					--Shape = Yutils.shape.flatten( Shape, 3 )
					Shape = ke4.shape.redraw( Shape, 3.15 * ratio, "bezier" )
					ke4.shape.info( Shape )
					local Shape2 = ke4.shape.displace( Shape, -shape_x2[ 1 ], -shape_y2[ 1 ] )
					local off_px, off_py = 0, 0
					if Relative then
						Shape2 = Shape
						off_px, off_py = shape_x2[ 1 ], shape_y2[ 1 ]
					end
					for i = 1, #ke4.shape.length( Shape2, "segments" ) do
						segms[ i ] = ke4.shape.length( Shape2, "segments" )[ i ]
						segms[ i ] = ke4.math.round( segms[ i ] )
					end
					local lengthx, times = ke4.shape.length( Shape2 ), { [ 0 ] = 0 }
					local posx = ke4.math.round( pos_x - val_width / 2 + off_px, 2 )
					local posy = ke4.math.round( pos_y + val_height / 2 - l_descent + off_py, 2 )
					local tags = format( "\\fscx%s\\fscy%s", posx, posy )
					for i = 1, #segms do
						times[ i ] = dur_t * ke4.math.length_bezier( segms[ i ] ) / lengthx + times[ i - 1 ]
					end
					times = ke4.math.round( ke4.table.op( times, "add", time1 ), 2 )
					times[ 0 ] = time1
					local function shape_lmove( Segment, time1, time2 )
						local size_fscx = ke4.math.round( pos_x - val_width / 2 + Segment[ 3 ], 2 )
						local size_fscy = ke4.math.round( pos_y + val_height / 2 - l_descent + Segment[ 4 ], 2 )
						return format( "\\t(%s,%s,1,\\fscx%s\\fscy%s)", time1, time2, size_fscx, size_fscy )
					end
					for k = 1, #segms do
						tags = tags .. shape_lmove( segms[ k ], times[ k - 1 ], times[ k ] )
					end
					tags2 = format( "%s\\p1}m 0 0 m 100 100 {\\p0\\r}", tags )
				--end
				local fscxy, sizex, sizey = { }, { }, { }
				for c in tags2:gmatch( "fscx?y?(%-?%d+[%.%d]*)" ) do
					table.insert( fscxy, tonumber( c ) )
				end
				for i = 1, #fscxy / 2 do
					table.insert( sizex, fscxy[ 2 * i - 1 ] )
					table.insert( sizey, fscxy[ 2 * i - 0 ] )
				end
				local min_x, min_y = ke4.table.op( sizex, "min" ), ke4.table.op( sizey, "min" )
				if min_x >= 0 then
					min_x = 0
				end
				if min_y > 0
					and min_y <= val_height then
					min_y = -min_y
				elseif min_y >= 0 then
					min_y = 0
				end
				local posxy = format( "\\an7\\q2\\pos(%s,%s)", min_x, min_y )
				tags2 = "{" .. posxy .. tags2:gsub( "(\\fscx%-?%d+[%.%d]*)(\\fscy%-?%d+[%.%d]*)", 
					function( x, y )
						fsc_x = x:match( "%-?%d+[%.%d]*" )
						fsc_y = y:match( "%-?%d+[%.%d]*" )
						return format( "\\fscx%s\\fscy%s", fsc_x - min_x, fsc_y - min_y )
					end
				)
				tags2 = tags2:gsub( "\\t%((%d+[%.%d]*)%,(%d+[%.%d]*)",
					function( t_ini, t_fin )
						if tonumber( t_fin ) > time2 then
							t_fin = time2
							return format( "\\t(%s,%s", t_ini, t_fin )
						end
					end
				)
				:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" )
				return tags2
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.smove( Configs, _G.ke4.shape.trebol, 0, line.duration )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!
			
			rmove = function( Configs, Rx, Ry, t1, t2, Accel, offset_t, Counter2 ) -- Random Move
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local l_descent = Configs[ 1 ].descent
				local pos_x, pos_y = Configs[ 2 ], Configs[ 3 ]
				local val_width, val_height = Configs[ 4 ], Configs[ 5 ]
				local offset_t = offset_t or 0
				local accel = Accel or 1
				local time2 = t2 or Configs[ 1 ].duration
				local time1 = t1 or 0
				local Rx = Rx or 0.4 * val_height
				local Ry = Ry or Rx
				local tgdur
				local dur_t = time2 - time1
				--local line = linefx[ ii ]
				local func_accel = loadstring( format( "return function( meta, line, i ) return %s end", accel ) )( )
				if type( offset_t ) == "table" then
					tgdur = ke4.math.round( offset_t[ 1 ], 2 )
				else
					tgdur = ke4.math.round( 3.6 * frame_dur + offset_t, 2 )
				end
				tgdur = abs( tgdur )
				local posx = ke4.math.round( pos_x - val_width / 2, 2 )
				local posy = ke4.math.round( pos_y + val_height / 2 - l_descent, 2 )
				local tags1, randx, randy, i = format( "\\fscx%s\\fscy%s", posx, posy ), 0, 0, 0
				local tm1, tm2, pdx, pdy
				if type( Rx ) == "string" then
					-- se mueve secuencialmente entre los puntos de la shape
					local coor, coor_x, coor_y = { }, { }, { }
					for num in Rx:gmatch( "%-?%d+[%.%d]*" ) do
						table.insert( coor, tonumber( num ) )
					end
					for i = 1, #coor/2 do
						coor_x[ i ] = coor[ 2 * i - 1 ]
						coor_y[ i ] = coor[ 2 * i - 0 ]
					end
					while dur_t > 0 do
						randx = coor_x[ (i + 1) % #coor_x + 1 ]
						randy = coor_y[ (i + 1) % #coor_y + 1 ]
						if dur_t - tgdur <= 0 then
							randx = 0
							randy = 0
						end
						tm1 = time1 + tgdur * (i + 0)
						tm2 = time1 + tgdur * (i + 1)
						if dur_t - tgdur <= 0 then
							tm2 = time2
						end
						pdx = posx + randx
						pdy = posy + randy
						accel = ke4.math.round( func_accel( meta, line, i ), 3 )
						tags1 = tags1 .. format( "\\t(%s,%s,%s,\\fscx%s\\fscy%s)", tm1, tm2, accel, pdx, pdy )
						i = i + 1
						dur_t = dur_t - tgdur
					end
				else
					while dur_t > 0 do
						if type( Rx ) == "table" then
							if type( Rx[ 1 ] ) == "table" then
								randx = tostring( Rx[ 1 ][ 1 ] ) .. ">" .. tostring( Rx[ 1 ][ 2 ] )
							else
								randx = ke4.math.R( Rx[ 1 ], Rx[ 2 ], 1 )
							end
						elseif type( Rx ) == "function" then
							Rx_func = Rx( )
							randx = ke4.math.R( -Rx_func, Rx_func, 1 )
						else
							randx = ke4.math.R( -Rx, Rx, 1 )
						end
						if type( Ry ) == "table" then
							if type( Ry[ 1 ] ) == "table" then
								randy = tostring( Ry[ 1 ][ 1 ] ) .. ">" .. tostring( Ry[ 1 ][ 2 ] )
							else
								randy = ke4.math.R( Ry[ 1 ], Ry[ 2 ], 1 )
							end
						elseif type( Ry ) == "function" then
							Ry_func = Ry( )
							randy = ke4.math.R( -Ry_func, Ry_func, 1 )
						else
							randy = ke4.math.R( -Ry, Ry, 1 )
						end
						if type( randx ) == "number" then
							if dur_t - tgdur <= 0 then
								randx = 0
							end
							pdx = posx + randx
						else
							pdx = tostring( posx ) .. "+" .. randx
						end
						if type( randy ) == "number" then
							if dur_t - tgdur <= 0 then
								randy = 0
							end
							pdy = posy + randy
						else
							pdy = tostring( posy ) .. "+" .. randy
						end
						tm1 = time1 + tgdur * (i + 0)
						tm2 = time1 + tgdur * (i + 1)
						if dur_t - tgdur <= 0 then
							tm2 = time2
						end
						accel = ke4.math.round( func_accel( meta, line, i ), 3 )
						tags1 = tags1 .. format( "\\t(%s,%s,%s,\\fscx%s\\fscy%s)", tm1, tm2, accel, pdx, pdy )
						i = i + 1
						dur_t = dur_t - tgdur
					end
				end
				local cap_nx, cap_ix = ke4.string.count( tags1, "(\\fscx)(%-?%d+[%.%d]*)%+(%-?%d+[%.%d]*)%>(%-?%d+[%.%d]*)" ), 0
				tags1 = tags1:gsub( "(\\fscx)(%-?%d+[%.%d]*)%+(%-?%d+[%.%d]*)%>(%-?%d+[%.%d]*)",
					function( Tag, Val0, Val1, Val2 )
						local Val0, Val1, Val2 = tonumber( Val0 ), tonumber( Val1 ), tonumber( Val2 )
						local Rank = Val2 - Val1
						cap_ix = cap_ix + 1
						return format( "%s%s", Tag, Val0 + Val1 + Rank * cap_ix / cap_nx )
					end
				)
				local cap_ny, cap_iy = ke4.string.count( tags1, "(\\fscy)(%-?%d+[%.%d]*)%+(%-?%d+[%.%d]*)%>(%-?%d+[%.%d]*)" ), 0
				tags1 = tags1:gsub( "(\\fscy)(%-?%d+[%.%d]*)%+(%-?%d+[%.%d]*)%>(%-?%d+[%.%d]*)",
					function( Tag, Val0, Val1, Val2 )
						local Val0, Val1, Val2 = tonumber( Val0 ), tonumber( Val1 ), tonumber( Val2 )
						local Rank = Val2 - Val1
						cap_iy = cap_iy + 1
						return format( "%s%s", Tag, Val0 + Val1 + Rank * cap_iy / cap_ny )
					end
				)
				local tags2 = format( "%s\\p1}m 0 0 m 100 100 {\\p0\\r}", tags1 )
				local fscxy, sizex, sizey = { }, { }, { }
				for c in tags2:gmatch( "fscx?y?(%-?%d+[%.%d]*)" ) do
					table.insert( fscxy, tonumber( c ) )
				end
				for i = 1, #fscxy / 2 do
					table.insert( sizex, fscxy[ 2 * i - 1 ] )
					table.insert( sizey, fscxy[ 2 * i - 0 ] )
				end
				local min_x, min_y = ke4.table.op( sizex, "min" ), ke4.table.op( sizey, "min" )
				if min_x >= 0 then
					min_x = 0
				end
				if min_y > 0
					and min_y <= val_height then
					min_y = -min_y
				elseif min_y >= 0 then
					min_y = 0
				end
				local posxy = format( "\\an7\\q2\\pos(%s,%s)", min_x, min_y )
				tags2 = "{" .. posxy .. tags2:gsub( "(\\fscx%-?%d+[%.%d]*)(\\fscy%-?%d+[%.%d]*)", 
					function( x, y )
						fsc_x = x:match( "%-?%d+[%.%d]*" )
						fsc_y = y:match( "%-?%d+[%.%d]*" )
						return format( "\\fscx%s\\fscy%s", fsc_x - min_x, fsc_y - min_y )
					end
				)
				tags2 = tags2:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" )
				return tags2
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.rmove( Configs, 10, 10, 0, line.duration )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!
			
			rmove2 = function( Configs, Rx, Ry, t, Accel ) -- Random Move
				--similar a rmove, pero con "shakes" a intervalos determinados
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local time_ini, time_fin
				local time_tag = ke4.math.round( 2.8 * frame_dur, 2 )			--Duración de cada transformación
				local time_shk = ke4.math.R( 3, 6 ) * time_tag					--Duración de cada "Shake"
				local time_off = ke4.math.R( 4, 8 ) * time_tag					--Duración del tiempo "off"
				if t then
					time_ini = t[ 1 ] or 0
					time_fin = t[ 2 ] or 5000--fx.dur
					time_tag = t[ 3 ] or ke4.math.round( 2.8 * frame_dur, 2 )	--Duración de cada transformación
					time_shk = t[ 4 ] or ke4.math.R( 3, 6 ) * time_tag			--Duración de cada "Shake"
					time_off = t[ 5 ] or ke4.math.R( 4, 8 ) * time_tag			--Duración del tiempo "off"
				end
				local accel = Accel or 1
				local time_dur = time_fin - time_ini
				local tag_tbl, tag_dur, i = { }, 0, 1
				local tag_tm1, tag_tm2, tag_shk, tag_off
				while time_dur > tag_dur do
					if type( Accel ) == "function" then
						accel = Accel( )
					end
					tag_shk = ke4.math.R( 0.75 * time_shk, 1.25 * time_shk, 1 )
					tag_off = ke4.math.R( 0.65 * time_off, 1.25 * time_off, 1 )
					tag_tm1 = tag_dur + ke4.tag.only( i == 1 and ke4.math.R( 2 ) == 2, 0, tag_off )
					tag_tm2 = tag_tm1 + tag_shk
					tag_tbl[ i ] = ke4.tag.rmove( Configs, Rx, Ry, tag_tm1, tag_tm2, Accel, { time_tag } )
					tag_dur = tag_tm2
					i = i + 1
				end
				local tag_fx1 = { }
				for i = 2, #tag_tbl do
					tag_fx1[ i - 1 ] = ""
					for c in tag_tbl[ i ]:gmatch( "\\t%b()" ) do
						tag_fx1[ i - 1 ] = tag_fx1[ i - 1 ] .. c
					end
				end
				local tag_fx2 = tag_tbl[ 1 ]
				local tag_fx3 = ke4.table.op( tag_fx1, "concat" )
				local tag_fx4 = tag_fx2:gsub( "\\p1", tag_fx3 .. "\\p1" )
				return tag_fx4
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.rmove2( Configs, 12, 12, { 0, line.duration } )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!
			
			rmove3 = function( Configs, Rx, Ry, t, Accel, offset_t )
				local times = t or { 0, 5000 }
				local rmove3_tgfx = ""
				local rmove3_tags = ke4.tag.rmove( Configs, Rx, Ry, times[ 1 ], times[ 2 ], Accel, offset_t )
				local rmove3_tag3 = { }
				local rmove3_tbl3 = { }
				local rmove3_accl = Accel
				local rmove3_offt = offset_t
				for i = 2, #times / 2 do
					if type( Accel ) == "function" then
						rmove3_accl = Accel( )
					end
					if type( offset_t ) == "function" then
						rmove3_offt = offset_t( )
					end
					rmove3_tbl3[ i - 1 ] = { }
					rmove3_tag3[ i - 1 ] = ke4.tag.rmove( Configs, Rx, Ry, times[ 2 * i - 1 ], times[ 2 * i ], rmove3_accl, rmove3_offt )
					for tr in rmove3_tag3[ i - 1 ]:gmatch( "\\t%(%d+[%.%d]*%,%d+[%.%d]*%,[%d%.%,]*\\fscx%d+[%.%d]*\\fscy%d+[%.%d]*%)" ) do
						table.insert( rmove3_tbl3[ i - 1 ], tr )
					end
					rmove3_tag3[ i - 1 ] = ke4.table.op( rmove3_tbl3[ i - 1 ], "concat" )
				end
				rmove3_tag3 = ke4.table.op( rmove3_tag3, "concat" )
				rmove3_tgfx = rmove3_tags:gsub( "\\p1", format( "%s\\p1", rmove3_tag3 ) )
				return rmove3_tgfx
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.rmove3( Configs, 12, 12, { 0, 500, line.duration - 500, line.duration }, 1, { 84 } )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!
			
			rmove4 = function( Configs, Rx, Ry, t1, t2, Accel, offset_t, move4 )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				----------------------------------------
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local time1 = t1 or 0--fx.movet_i
				local time2 = t2 or 5000--fx.movet_f
				local dur_t = time2 - time1
				local move4 = move4 or { 1.5 * frame_dur, 25 * ratio }
				move4[ 3 ] = move4[ 3 ] or 1
				local tag_r = ke4.tag.rmove( Configs, Rx, Ry, time1, time2, Accel, offset_t )
				local tagfx, Dur, i = "\\fscx0\\fscy0", dur_t, 0
				local t, dx, acc
				while Dur > 0 do
					if type( move4[ 1 ] ) == "table" then
						t = ke4.math.round( ke4.math.R( move4[ 1 ][ 1 ] * 100, move4[ 1 ][ 2 ] * 100, 1 ) / 100, 2 )
					else
						t = ke4.math.round( move4[ 1 ], 2 )
					end
					if type( move4[ 2 ] ) == "table" then
						dx = ke4.math.round( ke4.math.R( move4[ 2 ][ 1 ] * 100, move4[ 2 ][ 2 ] * 100, 1 ) / 100, 2 )
					else
						dx = ke4.math.round( move4[ 2 ], 2 )
					end
					if type( move4[ 3 ] ) == "table" then
						acc = ke4.math.round( ke4.math.R( move4[ 3 ][ 1 ] * 100, move4[ 3 ][ 2 ] * 100, 1 ) / 100, 2 )
					else
						acc = ke4.math.round( move4[ 3 ], 2 )
					end
					if dx < 0 or Dur - t <= 0 then
						dx = 0
					end
					tagfx = tagfx .. format( "\\t(%s,%s,%s,\\fscx%s)", time1 + i * t, time1 + (i + 1) * t, acc, dx * ((i + 1) % 2) )
					Dur = Dur - t
					i = i + 1
				end
				local dx2
				if type( move4[ 2 ] ) == "table" then
					dx2 = ke4.math.round( abs( (move4[ 2 ][ 2 ] - move4[ 2 ][ 1 ]) / 2 ), 2 )
				else
					dx2 = ke4.math.round( move4[ 2 ] / 2, 2 )
				end
				tag_r = tag_r:gsub( "\\pos%((%-?%d+[%.%d]*)%,(%-?%d+[%.%d]*)%)",
					function( x, y )
						return format( "\\pos(%s,%s)", tonumber( x ) - dx2, y )
					end
				)
				tagfx = "{" .. tagfx .. "\\p1}m 0 0 m 100 100 {\\p0\\r}"
				tagfx = tag_r:gsub( "\\r}", "\\r}" .. tagfx )
				tagfx = tagfx:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" ):gsub( "}{", "" )
				return tagfx
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.rmove4( Configs, 20, 20, 0, line.duration, 1, { 460 }, { 126, { 20, 40 } } )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!
			
			omove = function( Configs, P, t1, t2, Dur, Accel ) -- Oscill Move
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local l_descent = Configs[ 1 ].descent
				local pos_x, pos_y = Configs[ 2 ], Configs[ 3 ]
				local val_width, val_height = Configs[ 4 ], Configs[ 5 ]
				local Ocoor = P
				local time1 = ke4.math.round( t1 or 0, 2 )
				local time2 = ke4.math.round( t2 or 5000, 2 )
				local DurOM = time2 - time1
				local Durtt = Dur or 2 * frame_dur
				local accel = Accel or 1
				if type( Ocoor ) == "string" then
					local Ocoor2 = { }
					for num in Ocoor:gmatch( "%-?%d+[%.%d]*" ) do
						table.insert( Ocoor2, tonumber( num ) )
					end
					Ocoor = Ocoor2
				end
				local coorx, coory = { }, { }
				for i = 1, #Ocoor / 2 do
					coorx[ i ] = ke4.math.round( Ocoor[ 2 * i - 1 ], 2 )
					coory[ i ] = ke4.math.round( Ocoor[ 2 * i - 0 ], 2 )
				end
				--local line = linefx[ ii ]
				local func_accel = loadstring( format( "return function( meta, line, i ) return %s end", accel ) )( )
				local posx = ke4.math.round( pos_x - val_width / 2, 2 )
				local posy = ke4.math.round( pos_y + val_height / 2 - l_descent, 2 )
				local tags1, i = format( "\\fscx%s\\fscy%s", posx, posy ), 0
				local px, py, t1, t2
				while DurOM > 0 do
					px = coorx[ i % #coorx + 1 ]
					py = coory[ i % #coory + 1 ]
					t1 = ke4.math.round( time1 + Durtt * (i + 0), 2 )
					t2 = ke4.math.round( time1 + Durtt * (i + 1), 2 )
					if DurOM - Durtt <= 0 then
						t2 = time2 - time1
						px, py = 0, 0
					end
					accel = ke4.math.round( func_accel( meta, line, i ), 3 )
					tags1 = tags1 .. format( "\\t(%s,%s,%s,\\fscx%s\\fscy%s)", t1, t2, accel, posx + px, posy + py )
					i = i + 1
					DurOM = DurOM - Durtt
				end
				local tags2 = format( "%s\\p1}m 0 0 m 100 100 {\\p0\\r}", tags1 )
				local fscxy, sizex, sizey = { }, { }, { }
				for c in tags2:gmatch( "fscx?y?(%-?%d+[%.%d]*)" ) do
					table.insert( fscxy, tonumber( c ) )
				end
				for i = 1, #fscxy / 2 do
					table.insert( sizex, fscxy[ 2 * i - 1 ] )
					table.insert( sizey, fscxy[ 2 * i - 0 ] )
				end
				local min_x, min_y = ke4.table.op( sizex, "min" ), ke4.table.op( sizey, "min" )
				if min_x >= 0 then
					min_x = 0
				end
				if min_y > 0
					and min_y <= val_height then
					min_y = -min_y
				elseif min_y >= 0 then
					min_y = 0
				end
				local posxy = format( "\\an7\\q2\\pos(%s,%s)", min_x, min_y )
				tags2 = "{" .. posxy .. tags2:gsub("(\\fscx%-?%d+[%.%d]*)(\\fscy%-?%d+[%.%d]*)", 
					function( x, y )
						fsc_x = x:match( "%-?%d+[%.%d]*" )
						fsc_y = y:match( "%-?%d+[%.%d]*" )
						return format( "\\fscx%s\\fscy%s", fsc_x - min_x, fsc_y - min_y )
					end
				)
				tags2 = tags2:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" )
				return tags2
				--code line: Configs = { line, line.center, line.middle, line.width, line.height }
			end, --!_G.ke4.tag.omove( Configs, { 0, 30, 0, -30 }, 0, line.duration )!{\1c&HFF00FF&\fad(100,100)}!line.text_stripped!
		
			default = function( Text_Config, String )
				local l = ke4.table.duplicate( Text_Config.styleref )
				l.duration, l.center, l.middle = Text_Config.duration, Text_Config.center, Text_Config.middle
				local text_color1, text_color2 = ke4.color.fromstyle( l.color1 ), ke4.color.fromstyle( l.color2 )
				local text_color3, text_color4 = ke4.color.fromstyle( l.color3 ), ke4.color.fromstyle( l.color4 )
				local text_alpha1, text_alpha2 = ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color2 )
				local text_alpha3, text_alpha4 = ke4.alpha.fromstyle( l.color3 ), ke4.alpha.fromstyle( l.color4 )
				local tags_default = ""
				local Str_def = String or ""
				local tags_default_tags = {
					[ 01 ] = "\\1v?c",						[ 02 ] = "\\2v?c",						[ 03 ] = "\\3v?c",
					[ 04 ] = "\\4v?c",						[ 05 ] = "\\c&H",						[ 06 ] = "\\1v?a",
					[ 07 ] = "\\2v?a",						[ 08 ] = "\\3v?a",						[ 09 ] = "\\4v?a",
					[ 10 ] = "\\alpha",						[ 11 ] = "\\fsp",						[ 12 ] = "\\fsvp",
					[ 13 ] = "\\blur",						[ 14 ] = "\\fe",						[ 15 ] = "\\be",
					[ 16 ] = "\\rndx",						[ 17 ] = "\\rndy",						[ 18 ] = "\\rndz",
					[ 19 ] = "\\frs",						[ 20 ] = "\\xbord",						[ 21 ] = "\\ybord",
					[ 22 ] = "\\bord",						[ 23 ] = "\\fn",						[ 24 ] = "\\z",
					[ 25 ] = "\\xshad",						[ 26 ] = "\\yshad",						[ 27 ] = "\\shad",
					[ 28 ] = "\\fs%d+[%.%d]*",				[ 29 ] = "\\fs%(",						[ 30 ] = "\\fsR",
					[ 31 ] = "\\fsc%d+[%.%d]*",				[ 32 ] = "\\fsc%(",						[ 33 ] = "\\fscR",
					[ 34 ] = "\\rnd%-?%d+[%.%d]*",			[ 35 ] = "\\rnd%(",						[ 36 ] = "\\rndR",
					[ 37 ] = "\\fax%-?%d+[%.%d]*",			[ 38 ] = "\\fax%(",						[ 39 ] = "\\faxR",
					[ 40 ] = "\\fay%-?%d+[%.%d]*",			[ 41 ] = "\\fay%(",						[ 42 ] = "\\fayR",
					[ 43 ] = "\\frx[o]*%-?%d+[%.%d]*",		[ 44 ] = "\\frx[o]*%(",					[ 45 ] = "\\frx[o]*R",
					[ 46 ] = "\\fry[o]*%-?%d+[%.%d]*",		[ 47 ] = "\\fry[o]*%(",					[ 48 ] = "\\fry[o]*R",
					[ 49 ] = "\\fr[zo]*%-?%d+[%.%d]*",		[ 50 ] = "\\fr[zo]*%(",					[ 51 ] = "\\fr[zo]*R",
					[ 52 ] = "\\fscx[r]*%d+[%.%d]*",		[ 53 ] = "\\fscx[r]*%(",				[ 54 ] = "\\fscx[r]*R",
					[ 55 ] = "\\fscy[r]*%d+[%.%d]*",		[ 56 ] = "\\fscy[r]*%(",				[ 57 ] = "\\fscy[r]*R",
					[ 58 ] = "\\frxy[o]*%-?%d+[%.%d]*",		[ 59 ] = "\\frxy[o]*%(",				[ 60 ] = "\\frxy[o]*R",
					[ 61 ] = "\\frxz[o]*%-?%d+[%.%d]*",		[ 62 ] = "\\frxz[o]*%(",				[ 63 ] = "\\frxz[o]*R",
					[ 64 ] = "\\fryz[o]*%-?%d+[%.%d]*",		[ 65 ] = "\\fryz[o]*%(",				[ 66 ] = "\\fryz[o]*R",
					[ 67 ] = "\\faxy",						[ 68 ] = "\\frxyz",						[ 69 ] = "\\fscxy",
					[ 70 ] = "\\xyshad",					[ 71 ] = "\\xybord",
				}
				local tags_deafult_vals = {
					[ 01 ] = "\\1c" .. text_color1,			[ 02 ] = "\\2c" .. text_color2,			[ 03 ] = "\\3c" .. text_color3,
					[ 04 ] = "\\4c" .. text_color4,			[ 05 ] = "\\c" .. text_color1,			[ 06 ] = "\\1a" .. text_alpha1,
					[ 07 ] = "\\2a" .. text_alpha2,			[ 08 ] = "\\3a" .. text_alpha3,			[ 09 ] = "\\4a" .. text_alpha4,
					[ 10 ] = "\\alpha&H00&",				[ 11 ] = "\\fsp" .. l.spacing,			[ 12 ] = "\\fsvp0",
					[ 13 ] = "\\blur0",						[ 14 ] = "\\fe0",						[ 15 ] = "\\be0",
					[ 16 ] = "\\rndx0",						[ 17 ] = "\\rndy0",						[ 18 ] = "\\rndz0",
					[ 19 ] = "\\frs0",						[ 20 ] = "\\xbord" .. l.outline,		[ 21 ] = "\\ybord" .. l.outline,
					[ 22 ] = "\\bord" .. l.outline,			[ 23 ] = "\\fn" .. l.fontname,			[ 24 ] = "\\z0",
					[ 25 ] = "\\xshad" .. l.shadow,			[ 26 ] = "\\yshad" .. l.shadow,			[ 27 ] = "\\shad" .. l.shadow,
					[ 28 ] = "\\fs" .. l.fontsize,			[ 29 ] = "\\fs" .. l.fontsize,			[ 30 ] = "\\fs" .. l.fontsize,
					[ 31 ] = "\\fsc" .. l.scale_x,			[ 32 ] = "\\fsc" .. l.scale_x,			[ 33 ] = "\\fsc" .. l.scale_x,
					[ 34 ] = "\\rnd0",						[ 35 ] = "\\rnd0",						[ 36 ] = "\\rnd0",
					[ 37 ] = "\\fax0",						[ 38 ] = "\\fax0",						[ 39 ] = "\\fax0",
					[ 40 ] = "\\fay0",						[ 41 ] = "\\fay0",						[ 42 ] = "\\fay0",
					[ 43 ] = "\\frx0",						[ 44 ] = "\\frx0",						[ 45 ] = "\\frx0",
					[ 46 ] = "\\fry0",						[ 47 ] = "\\fry0",						[ 48 ] = "\\fry0",
					[ 49 ] = "\\frz" .. l.angle,			[ 50 ] = "\\frz" .. l.angle,			[ 51 ] = "\\frz" .. l.angle,
					[ 52 ] = "\\fscx" .. l.scale_x,			[ 53 ] = "\\fscx" .. l.scale_x,			[ 54 ] = "\\fscx" .. l.scale_x,
					[ 55 ] = "\\fscy" .. l.scale_y,			[ 56 ] = "\\fscy" .. l.scale_y,			[ 57 ] = "\\fscy" .. l.scale_y,
					[ 58 ] = "\\frx0\\fry0",				[ 59 ] = "\\frx0\\fry0",				[ 60 ] = "\\frx0\\fry0",
					[ 61 ] = "\\frx0\\frz" .. l.angle,		[ 62 ] = "\\frx0\\frz" .. l.angle,		[ 63 ] = "\\frx0\\frz" .. l.angle,
					[ 64 ] = "\\fry0\\frz" .. l.angle,		[ 65 ] = "\\fry0\\frz" .. l.angle,		[ 66 ] = "\\fry0\\frz" .. l.angle,
					[ 67 ] = "\\fax0\\fay0",				[ 68 ] = "\\frx0\\fry0\\frz" .. l.angle,
					[ 69 ] = format( "\\fscx%s\\fscy%s", l.scale_x, l.scale_y ),
					[ 70 ] = format( "\\xshad%s\\yshad%s", l.shadow, l.shadow ),
					[ 71 ] = format( "\\xbord%s\\ybord%s", l.outline, l.outline ),
				}
				local tabs_def = { }
				for i = 1, #tags_default_tags do
					Str_def = Str_def:gsub( tags_default_tags[ i ],
						function( tags )
							return "@" .. tags_deafult_vals[ i ] .. "@"
						end
					)
				end
				Str_def = Str_def:gsub( "%b@@",
					function( capture )
						table.insert( tabs_def, capture:sub( 2, -2 ) )
					end
				)
				if #tabs_def > 0 then
					tags_default = ke4.table.op( tabs_def, "concat" )
				end
				local function delete_repeat_tag( String )
					local tags_unrepeat_vals = {
						[ 01 ] = "\\1c" .. text_color1,		[ 02 ] = "\\2c" .. text_color2,		[ 03 ] = "\\3c" .. text_color3,
						[ 04 ] = "\\4c" .. text_color4,		[ 05 ] = "\\c" .. text_color1,		[ 06 ] = "\\1a" .. text_alpha1,
						[ 07 ] = "\\2a" .. text_alpha2,		[ 08 ] = "\\3a" .. text_alpha3,		[ 09 ] = "\\4a" .. text_alpha4,
						[ 10 ] = "\\alpha&H00&",			[ 11 ] = "\\fsp" .. l.spacing,		[ 12 ] = "\\xbord" .. l.outline,
						[ 13 ] = "\\ybord" .. l.outline,	[ 14 ] = "\\bord" .. l.outline,		[ 15 ] = "\\fn" .. l.fontname,
						[ 16 ] = "\\z0",					[ 17 ] = "\\xshad" .. l.shadow,		[ 18 ] = "\\yshad" .. l.shadow,
						[ 19 ] = "\\shad" .. l.shadow,		[ 20 ] = "\\fsc" .. l.scale_x,		[ 21 ] = "\\fscx" .. l.scale_x,
						[ 22 ] = "\\fscy" .. l.scale_y,		[ 23 ] = "\\fs" .. l.fontsize,		[ 24 ] = "\\frx0",
						[ 25 ] = "\\fry0",					[ 26 ] = "\\frz" .. l.angle,		[ 27 ] = "\\fsvp0",
						[ 28 ] = "\\blur0",					[ 29 ] = "\\fe0",					[ 30 ] = "\\be0",
						[ 31 ] = "\\rndx0",					[ 32 ] = "\\rndy0",					[ 33 ] = "\\rndz0",
						[ 34 ] = "\\frs0",					[ 35 ] = "\\fay0",					[ 36 ] = "\\rnd0",
						[ 37 ] = "\\fax0",
					}
					local nm = 0
					for i = 1, #tags_unrepeat_vals do
						String, nm = String:gsub( tags_unrepeat_vals[ i ], "%1" )
						String = String:gsub( tags_unrepeat_vals[ i ], "", nm - 1 )
					end
					return String
				end
				return delete_repeat_tag( tags_default )
			end,
			
			default2 = function( Text_Config, String )
				local l = ke4.table.duplicate( Text_Config.styleref )
				l.duration, l.center, l.middle = Text_Config.duration, Text_Config.center, Text_Config.middle
				local text_color1, text_color2 = ke4.color.fromstyle( l.color1 ), ke4.color.fromstyle( l.color2 )
				local text_color3, text_color4 = ke4.color.fromstyle( l.color3 ), ke4.color.fromstyle( l.color4 )
				local text_alpha1, text_alpha2 = ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color2 )
				local text_alpha3, text_alpha4 = ke4.alpha.fromstyle( l.color3 ), ke4.alpha.fromstyle( l.color4 )
				local String = String or ""
				local tags_default_tags = {
					[ 01 ] = "\\1v?c[tcfswmd]*%~",			[ 02 ] = "\\2v?c[tcfswmd]*%~",			[ 03 ] = "\\3v?c[tcfswmd]*%~",
					[ 04 ] = "\\4v?c[tcfswmd]*%~",			[ 05 ] = "\\c[tcfswmd]*%~",				[ 06 ] = "\\1v?a[tcfswmd]*%~",
					[ 07 ] = "\\2v?a[tcfswmd]*%~",			[ 08 ] = "\\3v?a[tcfswmd]*%~",			[ 09 ] = "\\4v?a[tcfswmd]*%~",
					[ 10 ] = "\\alpha[tcfswmd]*%~",			[ 11 ] = "\\fsp[tcfswmd]*%~",			[ 12 ] = "\\fsvp[tcfswmd]*%~",
					[ 13 ] = "\\blur[tcfswmd]*%~",			[ 14 ] = "\\fe[tcfswmd]*%~",			[ 15 ] = "\\be[tcfswmd]*%~",
					[ 16 ] = "\\rndx[tcfswmd]*%~",			[ 17 ] = "\\rndy[tcfswmd]*%~",			[ 18 ] = "\\rndz[tcfswmd]*%~",
					[ 19 ] = "\\rnd[tcfswmd]*%~",			[ 20 ] = "\\frs[tcfswmd]*%~",			[ 21 ] = "\\fn[tcfswmd]*%~",
					[ 22 ] = "\\fs[tcfswmd]*%~",			[ 23 ] = "\\fsc[tcfswmd]*%~",			[ 24 ] = "\\z[tcfswmd]*%~",
					[ 25 ] = "\\bord[tcfswmd]*%~",			[ 26 ] = "\\xbord[tcfswmd]*%~",			[ 27 ] = "\\ybord[tcfswmd]*%~",
					[ 28 ] = "\\shad[tcfswmd]*%~",			[ 29 ] = "\\xshad[tcfswmd]*%~",			[ 30 ] = "\\yshad[tcfswmd]*%~",
					[ 31 ] = "\\fax[tcfswmd]*%~",			[ 32 ] = "\\fay[tcfswmd]*%~",			[ 33 ] = "\\faxy[tcfswmd]*%~",
					[ 34 ] = "\\frx[tcfswmd]*%~",			[ 35 ] = "\\fry[tcfswmd]*%~",			[ 36 ] = "\\fr[z]*[tcfswmd]*%~",
					[ 37 ] = "\\frxy[tcfswmd]*%~",			[ 38 ] = "\\frxz[tcfswmd]*%~",			[ 39 ] = "\\fryz[tcfswmd]*%~",
					[ 40 ] = "\\fscx[tcfswmd]*%~",			[ 41 ] = "\\fscy[tcfswmd]*%~",			[ 42 ] = "\\frxyz[tcfswmd]*%~",
					[ 43 ] = "\\fscxy[tcfswmd]*%~",			[ 44 ] = "\\xyshad[tcfswmd]*%~",		[ 45 ] = "\\xybord[tcfswmd]*%~",
					[ 46 ] = "\\bs[tcfswmd]*%~",
				}
				local tags_deafult_vals = {
					[ 01 ] = "\\1c%s" .. text_color1,		[ 02 ] = "\\2c%s" .. text_color2,			[ 03 ] = "\\3c%s" .. text_color3,
					[ 04 ] = "\\4c%s" .. text_color4,		[ 05 ] = "\\c%s" .. text_color1,			[ 06 ] = "\\1a%s" .. text_alpha1,
					[ 07 ] = "\\2a%s" .. text_alpha2,		[ 08 ] = "\\3a%s" .. text_alpha3,			[ 09 ] = "\\4a%s" .. text_alpha4,
					[ 10 ] = "\\alpha%s&H00&",				[ 11 ] = "\\fsp%s" .. l.spacing,			[ 12 ] = "\\fsvp%s0",
					[ 13 ] = "\\blur%s0",					[ 14 ] = "\\fe%s0",							[ 15 ] = "\\be%s0",
					[ 16 ] = "\\rndx%s0",					[ 17 ] = "\\rndy%s0",						[ 18 ] = "\\rndz%s0",
					[ 19 ] = "\\rnd%s0",					[ 20 ] = "\\frs%s0",						[ 21 ] = "\\fn%s" .. l.fontname,
					[ 22 ] = "\\fs%s" .. l.fontsize,		[ 23 ] = "\\fsc%s" .. l.scale_x,			[ 24 ] = "\\z%s0",
					[ 25 ] = "\\bord%s" .. l.outline,		[ 26 ] = "\\xbord%s" .. l.outline,			[ 27 ] = "\\ybord%s" .. l.outline,
					[ 28 ] = "\\shad%s" .. l.shadow,		[ 29 ] = "\\xshad%s" .. l.shadow,			[ 30 ] = "\\yshad%s" .. l.shadow,
					[ 31 ] = "\\fax%s0",					[ 32 ] = "\\fay%s0",						[ 33 ] = "\\fax%s0\\fay%s0",
					[ 34 ] = "\\frx%s0",					[ 35 ] = "\\fry%s0",						[ 36 ] = "\\frz%s" .. l.angle,
					[ 37 ] = "\\frx%s0\\\\fry%s0",			[ 38 ] = "\\frx%s0\\\\frz%s" .. l.angle,	[ 39 ] = "\\fry%s0\\\\frz%s" .. l.angle,
					[ 40 ] = "\\fscx%s" .. l.scale_x,		[ 41 ] = "\\fscy%s" .. l.scale_y,			[ 42 ] = "\\frx%s0\\\\fry%s0\\\\frz%s" .. l.angle,
					[ 43 ] = "\\fscx%s" .. l.scale_x .. "\\\\fscy%s" .. l.scale_y,
					[ 44 ] = "\\xshad%s" .. l.shadow .. "\\\\yshad%s" .. l.shadow,
					[ 45 ] = "\\xbord%s" .. l.outline .. "\\\\ybord%s" .. l.outline,
					[ 46 ] = "\\bord%s" .. l.outline .. "\\\\shad%s" .. l.shadow,
				} --!_G.ke4.tag.dark( line, "\\frR( 25 )" )!
				for i = 1, #tags_default_tags do
					String = String:gsub( tags_default_tags[ i ],
						function( capture_def )
							local cap = capture_def:gsub( "(\\%w+)(t[cfswmd]*)(%~)",
								function( Tag, Add_t, Sign )
									return format( tags_deafult_vals[ i ], Add_t, Add_t, Add_t )
								end
							)
							if cap ~= capture_def then
								return cap
							end
							local Add_t = ""
							return format( tags_deafult_vals[ i ], Add_t, Add_t, Add_t )
						end
					)
				end
				return String
			end,
			
			inverse = function( Text_Config, String )
				local l = ke4.table.duplicate( Text_Config.styleref )
				l.duration, l.center, l.middle = Text_Config.duration, Text_Config.center, Text_Config.middle
				local text_color1, text_color2 = ke4.color.fromstyle( l.color1 ), ke4.color.fromstyle( l.color2 )
				local text_color3, text_color4 = ke4.color.fromstyle( l.color3 ), ke4.color.fromstyle( l.color4 )
				local text_alpha1, text_alpha2 = ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color2 )
				local text_alpha3, text_alpha4 = ke4.alpha.fromstyle( l.color3 ), ke4.alpha.fromstyle( l.color4 )
				local String = String or ""
				local table_inverse = { }
				local tags_inverses = {
					[ 01 ] = "(\\[xy]*bord)(%-?%d+[%.%d]*)",	[ 02 ] = "(\\[xy]*shad)(%-?%d+[%.%d]*)",
					[ 03 ] = "(\\blur)(%-?%d+[%.%d]*)",			[ 04 ] = "(\\[%d]*c)(&H%x+&)",
					[ 05 ] = "(\\%da)(&H%x+&)",					[ 06 ] = "(\\alpha)(&H%x+&)",
					[ 07 ] = "(\\%da)(%d+[%.%d]*)",				[ 08 ] = "(\\alpha)(%d+[%.%d]*)",
					[ 09 ] = "(\\fsc[xy]*)(%-?%d+[%.%d]*)",		[ 10 ] = "(\\fr[sxyz]*)(%-?%d+[%.%d]*)",
					[ 11 ] = "(\\fa[xy]^*)(%-?%d+[%.%d]*)",		[ 13 ] = "(\\fs[v]*p)(%-?%d+[%.%d]*)",
					[ 13 ] = "(\\be)(%-?%d+[%.%d]*)",			[ 14 ] = "(\\z)(%-?%d+[%.%d]*)",
					[ 15 ] = "(\\rnd[xyz]*)(%-?%d+[%.%d]*)",
				}
				for i = 1, #tags_inverses do
					String = String:gsub( tags_inverses[ i ],
						function( tag_rev, val_rev )
							local l = Text_Config.styleref
							local tag_inv = tag_rev .. val_rev
							if tag_inv:match( "\\[%d]*c&H%x+&" ) then
								local r_nor, g_nor, b_nor = tag_inv:match( "(%x%x)(%x%x)(%x%x)" )
								local r_inv, g_inv, b_inv = 255 - tonumber( r_nor, 16 ), 255 - tonumber( g_nor, 16 ), 255 - tonumber( b_nor, 16 )
								return "<" .. tag_rev .. ke4.color.val2ass( b_inv, g_inv, r_inv ) .. ">"
							elseif tag_inv:match( "\\%da&H%x+&" )
								or tag_inv:match( "\\%da%d+[%.%d]*" ) then
								local num_alpha = tag_inv:match( "\\(%d)" )
								return "<" .. tag_rev .. loadstring( "return text_alpha" .. num_alpha )( ) .. ">"
							elseif tag_inv:match( "\\alpha&H%x+&" )
								or tag_inv:match( "\\alpha%d+[%.%d]*" ) then
								return "<" .. tag_rev .. "&H00&>"
							elseif tag_inv:match( "\\blur%-?%d+[%.%d]*" )
								or tag_inv:match( "\\[xy]*bord%-?%d+[%.%d]*" ) then
								return "<" .. tag_rev .. "0>"
							elseif tag_inv:match( "\\fsc[xy]^*%-?%d+[%.%d]*" ) then
								local scale_axis = tag_inv:match( "\\fsc([xy]^*)%-?%d+[%.%d]*" )
								if scale_axis == "x" then
									return "<" .. tag_rev .. tostring( l.scale_y ) .. ">"
								end
								return "<" .. tag_rev .. tostring( l.scale_x ) .. ">"
							elseif tag_inv:match( "\\fsc%-?%d+[%.%d]*" ) then
								return "<" .. tag_rev .. tostring( l.scale_x ) .. ">"
							else
								local val_inv = val_rev:gsub( "%-", "" )
								if val_rev:match( "%-" ) == nil then
									val_inv = "-" .. val_rev
								end
								return "<" .. tag_rev .. val_inv .. ">"
							end
						end
					)
				end
				String = String:gsub( "%b<>",
					function( capture )
						table.insert( table_inverse, capture:sub( 2, -2 ) )
					end
				)
				local tags_inverse_str, nm = "", 0
				if #table_inverse > 0 then
					tags_inverse_str = ke4.table.op( table_inverse, "concat" )
				end
				local tags_inverses2 = {
					[ 01 ] = "(\\[x]*bord)(%-?%d+[%.%d]*)",		[ 02 ] = "(\\ybord)(%-?%d+[%.%d]*)",
					[ 03 ] = "(\\[x]*shad)(%-?%d+[%.%d]*)",		[ 04 ] = "(\\yshad)(%-?%d+[%.%d]*)",
					[ 05 ] = "(\\blur)(%-?%d+[%.%d]*)",			[ 06 ] = "(\\[1]*c)(&H%x+&)",
					[ 07 ] = "(\\2c)(&H%x+&)",					[ 08 ] = "(\\3c)(&H%x+&)",
					[ 09 ] = "(\\4c)(&H%x+&)",					[ 10 ] = "(\\1a)(&H%x+&)",
					[ 11 ] = "(\\2a)(&H%x+&)",					[ 12 ] = "(\\3a)(&H%x+&)",
					[ 13 ] = "(\\4a)(&H%x+&)",					[ 14 ] = "(\\1a)(%d+[%.%d]*)",
					[ 15 ] = "(\\2a)(%d+[%.%d]*)",				[ 16 ] = "(\\3a)(%d+[%.%d]*)",
					[ 17 ] = "(\\4a)(%d+[%.%d]*)",				[ 18 ] = "(\\alpha)(&H%x+&)",
					[ 19 ] = "(\\alpha)(%d+[%.%d]*)",			[ 20 ] = "(\\fsc[x]*)(%-?%d+[%.%d]*)",
					[ 21 ] = "(\\fscy)(%-?%d+[%.%d]*)",			[ 22 ] = "(\\frs)(%-?%d+[%.%d]*)",
					[ 23 ] = "(\\frx)(%-?%d+[%.%d]*)",			[ 24 ] = "(\\fry)(%-?%d+[%.%d]*)",
					[ 25 ] = "(\\fr[z]*)(%-?%d+[%.%d]*)",		[ 26 ] = "(\\fax)(%-?%d+[%.%d]*)",
					[ 27 ] = "(\\fay)(%-?%d+[%.%d]*)",			[ 28 ] = "(\\fsp)(%-?%d+[%.%d]*)",
					[ 29 ] = "(\\fsvp)(%-?%d+[%.%d]*)",			[ 30 ] = "(\\be)(%-?%d+[%.%d]*)",
					[ 31 ] = "(\\z)(%-?%d+[%.%d]*)",			[ 32 ] = "(\\rnd)(%-?%d+[%.%d]*)",
					[ 33 ] = "(\\rndx)(%-?%d+[%.%d]*)",			[ 34 ] = "(\\rndy)(%-?%d+[%.%d]*)",
					[ 35 ] = "(\\rndz)(%-?%d+[%.%d]*)",
				}
				for i = 1, #tags_inverses2 do
					tags_inverse_str, nm = tags_inverse_str:gsub( tags_inverses2[ i ], "%1%2" )
					tags_inverse_str = tags_inverse_str:gsub( tags_inverses2[ i ], "", nm - 1 )
				end --"\\t4(\\frx56\\frz-89\\fry7)"
				return tags_inverse_str
			end,
			
			redefine = function( Text_Config, String )
				local l = ke4.table.duplicate( Text_Config.styleref )
				l.duration, l.center, l.middle = Text_Config.duration, Text_Config.center, Text_Config.middle
				-- "\\[%d]*%a+(%-?[%d&#]^*[%.%dH&%x]*)" --> captura un número/color
				--▼--------------------------------------------------------
				local String = String or ""
				String = ke4.tag.tonumber( String )	-- corvierte algunas convenciones en número
				--▼--------------------------------------------------------
				-- retorna los tags con sus valores en default: "\\blur~"
				String = String:gsub( "\\(%d%d+)(v?[ac]^*)%~",
					function( Coloralpha, Tag )
						local n = Coloralpha:len( )
						local newcap = ""
						for i = 1, n do
							newcap = newcap .. "\\" .. Coloralpha:sub( i, i ) .. Tag .. "~"
						end
						return newcap
					end
				)
				String = ke4.tag.default2( Text_Config, String )
				--▼--------------------------------------------------------
				-- separa los tags de los paréntesis que terminan en [-~]^*
				String = String:gsub( "(\\%w+%-?[%d&#]^*[%.%dH&%x]*)(%b()[%-%~]^*)", "%1 %2" )
				--▼--------------------------------------------------------
				-- encierra la funcción R dentro de paréntesis
				String = String:gsub( "R[%w]*%b()", "(%1)" )
				--▼--------------------------------------------------------
				-- tag.only
				String = ke4.string.cap( String, "\\(%b[])[ ]*(%b[])[ ]*", { "(%b[])" },
					function( condition, true_exit, false_exit )
						local condition = condition:sub( 2, -2 ):gsub( "\\", "\\\\" )
						local true_exit = ke4.string.toval( true_exit:sub( 2, -2 ) ):gsub( "\\", "\\\\" ):gsub( "%&", "↓" )
						if false_exit then
							false_exit = ke4.string.toval( false_exit:sub( 2, -2 ) ):gsub( "\\", "\\\\" ):gsub( "%&", "↓" )
						end
						local tag_only = format( "tag.only( %s, %s )", condition, "\"" .. true_exit .. "\"" )
						if false_exit then
							tag_only = format( "tag.only( %s, %s, %s )",
								condition, "\"" .. true_exit .. "\"", "\"" .. false_exit .. "\""
							) --"\\[j == 1][\\bs~]"
						end --"\\[syl.i == 1][\\1a0]"
						tag_only = ke4.string.toval( tag_only ):gsub( "↓", "%&" )
						return tag_only
					end --"\\[syl.i % 2 == 1][\\fr45][\\fr-45]"
				)	--"\\[syl.i == 1][\\fscy300]", "\\[syl.i > 5][\\1cR( )]", "\\[syl.i % 2 == 1][\\fr15][\\fr-15]"
				--▼--------------------------------------------------------
				-- da valores a los times y accel de una transformación \\t
				-- convierte los formatos de tiempos dentro de las transfos
				String = String:gsub( "\\t[%w%~%-]*%b()",
					function( capture )
						local capture = capture:gsub( "%d+%:%d+%:%d+%.%d+",
							function( cap_time )
								return tostring( ke4.time.HMS_to_ms( cap_time ) )
							end
						) --"\\tr(set,0:01:00.018,0:01:07.032,\\tags)"
						if capture:match( "\\t[%w%~%-]*%([ ]*\\" ) then
							return capture
						--[[
						elseif capture:match( "\\t[%w%~%-]*%(([^\\]*)" ) then
							timest_tbl = "{" .. capture:match( "\\t[%w%~%-]*%(([^\\]*)" ):sub( 1, -2 ) .. "}"
							timest_tbl = ke4.string.toval( timest_tbl )
							if type( timest_tbl ) == "table" then
								capture = capture:gsub( "(\\t[%w%~%-]*%()([^\\]*)",
									function( Pretag, Timestag )
										if Pretag:match( "\\tr" ) == nil then
											local time_ini = timest_tbl[ 1 ] or 0
											local time_acc = timest_tbl[ 3 ] or 1
											local time_fin
											if timest_tbl[ 2 ] == nil then
												time_ini, time_fin = 0, l.duration
												time_acc = timest_tbl[ 1 ]
											else
												time_fin = timest_tbl[ 2 ]
											end
											return format( "%s%s,%s,%s,\\", Pretag, time_ini, time_fin, time_acc )
										end
										return Pretag .. Timestag
									end
								)
								return capture
							end --"\\t(0.8,\\tags\\otros)"
							--]]
						end
						return capture
					end --"\\t( R( 100, 300, 10 ), 500 + line.n,\\xshadR( 4 ) )"
				)
				--▼--------------------------------------------------------
				-- saca algunos tags al inicio del \\t a una transfo instantánea
				String = String:gsub( "\\t[%w%~%-]*%b()",
					function( capture )
						local capture = ke4.string.coupling( capture, "(\\[%d]*%a+)(__)(:)(__)" )
						if capture:match( "\\[%d]*%a+%b()%:%b()" ) then
							local time_ini, time_fin = 0, 0
							if capture:match( "\\t[%w%~%-]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
								time_ini, time_fin = capture:match( "\\t[%w%~%-]*%([ ]*(%-?%d+[%.%d ]*),[ ]*(%-?%d+[%.%d ]*)," )
							end
							if capture:match( "\\te" ) then
								local time_dur = l.duration
								--[[
								if capture:match( "\\t[%w%~%-]*" ):match( "s" ) then
									time_dur = syl.end_time
								elseif capture:match( "\\t[%w%~%-]*" ):match( "c" ) then
									time_dur = char.end_time
								elseif capture:match( "\\t[%w%~%-]*" ):match( "f" ) then
									time_dur = furi.end_time
								elseif capture:match( "\\t[%w%~%-]*" ):match( "w" ) then
									time_dur = word.end_time
								end
								--]]
								time_ini = time_dur - time_fin
							end
							local tags_ini_tbl = { }
							for cap in capture:gmatch( "\\[%d]*%a+%b()%:%b()" ) do
								table.insert( tags_ini_tbl, cap:match( "(\\[%d]*%a+%b())%:%b()" ) )
							end
							---------------------------------------------------
							capture = capture:gsub( "\\[%d]*%a+(%b())%:(%b())",
								function( val_ini, val_fin )
									if ke4.string.toval( val_ini ) == ke4.string.toval( val_fin ) then
										return ""
									end
								end
							)
							---------------------------------------------------
							capture = capture:gsub( "(\\[%d]*%a+)%b()%:(%b())", "%1%2" )
							---------------------------------------------------
							if capture:match( "\\t[%w%~%-]*%(%-?%d[%.%d]*,%-?%d[%.%d]*,%-?%d[%.%d]*,\\%)" )
								or capture:match( "\\t[%w%~%-]*%(%-?%d[%.%d]*,%-?%d[%.%d]*,\\%)" ) then
								capture = "" -- para transfos vacías :)
							end
							---------------------------------------------------
							return format( "\\t(%s,%s,%s)%s", time_ini, time_ini, ke4.table.op( tags_ini_tbl, "concat" ), capture )
						end --"\\tes(0,100,\\frR( 45 ):0)"
					end --"\\tr(syl,\\fscxyr1.5:1\\1cR( ):TC1)"
				) --"\\t(300,900,0.8,\\1cR( ):&HFFFFFF&\\bs0\\blur1:3)"
				--▼--------------------------------------------------------
				-- convierte un \\t normal en una por default \\t --> \\td
				String = String:gsub( "\\t(%b()%b())", "\\td%1" )
				--▼--------------------------------------------------------
				--String = tag.natsu( String )
				--▼--------------------------------------------------------
				String = String:gsub( "(\\[%d+]*ct[cfsw]*)(%d+[%.%d]*)(d)(%(R%b()%))",
					function( cap1, cap2, cap3, cap4 )
						return cap1 .. "(" .. cap2 ..")" .. cap3 .. cap4:sub( 2, -2 )
					end
				) --"\\1ct300dR( )"
				--▼--------------------------------------------------------
				local function default_time( Stringt )
					-- pasa a un tag dentro de una transformación, luego
					-- en otra transformación le da su valor por default
					--▼ modifica momentáneamente algunos tags del VSFilterMod ---
					Stringt = Stringt:gsub( "(\\distort)(%b())", "\\disTorT%2" )	-- "\\distort(1,1,1,1)"
					:gsub( "(\\distort)(t[cdefimsw]*%b())", "\\disTorT%2" )
					:gsub( "(\\jitter)(%b())", "\\jiTTer%2" )
					:gsub( "(\\jitter)(t[cdefimsw]*%b())", "\\jiTTer%2" )
					--▼ encierra en paréntesis el valor numérico del tag \\t ----
					Stringt = Stringt:gsub( "(\\%w+)(t[cfsw]*)(%d+[f%.%d]*)(d)", "%1%2(%3)%4" )
					--▼ encierra en paréntesis el valor que se añadirá al tag ---
					Stringt = Stringt:gsub( "(\\%w+)(t[cfsw]*)(%b())(d)(&H%x+&)", "%1%2(%3)%4(%5)" )	-- ass colors and alphas
					:gsub( "(\\%w+)(t[cfsw]*)(%b())(d)([ACST]*%-?%d+[%.%d]*)", "%1%2(%3)%4(%5)" )		-- números y/o referencias de alpha/colores
					:gsub( "(\\%w+)(t[cfsw]*)(%b())(d)(R[cdemrst]*%b())", "%1%2(%3)%4(%5)" )			-- valores randoms, función random
					--▼ genera los remplazos, ingresa el tag dentro del \\t -----
					Stringt = Stringt:gsub( "(\\%w+)(t[cfsw]*)(%b())d(%b())",
						function( tag_in_time, tag_time, duration, valors )
							local duration = tostring( duration ):sub( 2, -2 )
							local time_dur = l.duration
							--[[
							if tag_time == "ts" then
								time_dur = syl.dur
							elseif tag_time == "tc" then
								time_dur = char.dur
							elseif tag_time == "tf" then
								time_dur = furi.dur
							elseif tag_time == "tw" then
								time_dur = word.dur
							end
							--]]
							if type( ke4.string.toval( duration ) ) == "number" then
								duration = ke4.string.toval( duration )
								return format( "\\%s(0,%s,%s)(0,%s)", tag_time, duration, tag_in_time .. valors, time_dur - duration )
								:gsub( "\\t%(", "\\td%(" )
							end
							return tag_in_time .. "(" .. duration .. ")d" .. valors
						end --"\\frxyt300dRs( 10 )"
					) --"\\frxyzt( 7f )dRs( 60, 90 )"
					--▼ opción cuando el tag \\t no lleva tiempos ---------------
					--▼ encierra en paréntesis el valor que se añadirá al tag ---
					Stringt = Stringt:gsub( "(\\%w+t[cdefimsw]*)([ACST]*%-?%d+[f%.%d]*)", "%1(%2)" )
					:gsub( "(\\%w+t[cdefimsw]*)(R[cdemrst]*%b())", "%1(%2)" )
					:gsub( "(\\%w+t[cdefimsw]*)(&H%x+&)", "%1(%2)" )
					--▼ genera los remplazos, ingresa el tag dentro del \\t -----
					Stringt = Stringt:gsub( "(\\%w+)(t[cdefimsw]*)(%b())",
						function( tag_in, tag_t, valors )
							return format( "\\%s(%s%s)", tag_t, tag_in, valors )
						end
					) --"\\1ctR( )" -->\\t(\\1cR( ))
					--▼ tm, tmd = time mid, time mid default --------------------
					Stringt = Stringt:gsub( "(\\tm[d]*)(%b())",
						function( tag_time, tags_in_time )
							tags_in_time = tags_in_time:sub( 2, -2 )
							tags_in_time = format( "(0,%s,%s)", l.duration / 2, tags_in_time )
							if tag_time:match( "d" ) then
								return format( "\\td%s(0,%s)", tags_in_time, l.duration / 2 )
							end
							return "\\t" .. tags_in_time
						end --"\\fscytm180" -->\\t(0,line.duration/2,\\fscy180)
					) --"\\fscytmd200" -->\\t(0,line.duration/2,\\fscy200)\\t(line.duration/2,line.duration,\\fscy100)
					return Stringt
				end
				--▼--------------------------------------------------------
				-- ke4.tag.oscill
				String = String:gsub( "(\\[%d]*%a+)(%-?[%d&]^*[%.%d&H%x]*)(o)(%d+[%.%d]*)", "%1(%2)%3(%4)" )
				:gsub( "(\\[%d]*%a+)(%-?[%d&]^*[%.%d&H%x]*)(o)(%b())", "%1(%2)%3%4" )
				:gsub( "(\\[%d]*%a+)(%b())(o)(%d+[%.%d]*)", "%1%2%3(%4)" )
				:gsub( "(\\[%d]*%a+)(%b())(o)(%b())(%d+[%.%d]*)", "%1%2%3%4(%5)" )
				String = String:gsub( "\\([%d]*%a+)(%b())o(%b())(%b())",
					function( Tag, Val, Dur, Delay )
						local TagVal = "\\\\" .. Tag .. Val
						local Dur = Dur:sub( 2, -2 )
						local Delay = Delay:sub( 2, -2 )
						local new_oscill = format( "ke4.tag.oscill( %s, %s, \"%s\" )", Dur, Delay, TagVal )
						return ke4.string.toval( new_oscill )
					end
				) --"\\frRs( 20 )o( 320 )( 80 )"
				String = String:gsub( "\\([%d]*%a+)(%b())o(%b())",
					function( Tag, Val, Delay )
						local TagVal = "\\\\" .. Tag .. Val
						local Dur = l.duration
						local Delay = Delay:sub( 2, -2 )
						local new_oscill = format( "ke4.tag.oscill( %s, %s, \"%s\" )", Dur, Delay, TagVal )
						return ke4.string.toval( new_oscill )
					end
				)
				--▼--------------------------------------------------------
				local function multi_alphas( String )
					local alphas = {
						[ 1 ] = "\\(%d%d+)a(%b())([%-%~]*)",
						[ 2 ] = "\\(%d%d+)a(&H%x+&)([%-%~]*)",
						[ 3 ] = "\\(%d%d+)a(%d+[%.%d]*)([%-%~]*)",
						------------------------------------------
						[ 4 ] = "\\(%d%d+)a(%b())([%-%~]^*%d+[%.%d]*)",
						[ 5 ] = "\\(%d%d+)a(&H%x+&)([%-%~]^*%d+[%.%d]*)",
						[ 6 ] = "\\(%d%d+)a(%d+[%.%d]*)([%-%~]^*%d+[%.%d]*)",
						------------------------------------------
						[ 7 ] = "\\(%d%d+)a(%b())([%-%~]^*%b())",
						[ 8 ] = "\\(%d%d+)a(&H%x+&)([%-%~]^*%b())",
						[ 9 ] = "\\(%d%d+)a(%d+[%.%d]*)([%-%~]^*%b())",
					}
					for i = 1, #alphas do
						String = String:gsub( alphas[ i ],
							function( nums, val, sign_val )
								local str_alp = ""
								local tbl_num = ke4.table.string( tostring( nums ) )
								for k = 1, #tbl_num do
									str_alp = str_alp .. format( "\\%sa%s", tbl_num[ k ], val .. sign_val )
								end
								return "@" .. str_alp .. "@"
							end		
						)
					end
					return String
				end --"\\34a200"
				--▼--------------------------------------------------------
				local function multi_colors( String )
					local String = String:gsub( "\\(%d+)([ac]^*[km]^*[^\\]*)",
						function( numbers, values )
							if numbers:len( ) > 1 then
								local coloralpha = ""
								for i = 1, numbers:len( ) do
									coloralpha = coloralpha .. "\\" .. numbers:sub( i, i ) .. values
								end
								return coloralpha
							end
						end
					) --"\\134amrR( 100, 200 )"
					local colors = {
						[ 1 ] = "\\(%d%d+)c(%b())([%-%~]*)",
						[ 2 ] = "\\(%d%d+)c(&H%x+&)([%-%~]*)",
						[ 3 ] = "\\(%d%d+)c(R%b())([%-%~]*)",
						------------------------------------------
						[ 4 ] = "\\(%d%d+)c(%b())([%-%~]^*%d+[%.%d]*)",
						[ 5 ] = "\\(%d%d+)c(&H%x+&)([%-%~]^*%d+[%.%d]*)",
						[ 6 ] = "\\(%d%d+)c(R%b())([%-%~]^*%d+[%.%d]*)",
						------------------------------------------
						[ 7 ] = "\\(%d%d+)c(%b())([%-%~]^*%b())",
						[ 8 ] = "\\(%d%d+)c(&H%x+&)([%-%~]^*%b())",
						[ 9 ] = "\\(%d%d+)c(R%b())([%-%~]^*%b())",
					}
					for i = 1, #colors do
						String = String:gsub( colors[ i ],
							function( nums, val, sign_val )
								local str_col = ""
								local tbl_num = ke4.table.string( tostring( nums ) )
								for k = 1, #tbl_num do
									str_col = str_col .. format( "\\%sc%s", tbl_num[ k ], val .. sign_val )
								end
								return "@" .. str_col .. "@"
							end		
						)
					end
					return String
				end --"\\34cR( )"
				--▼--------------------------------------------------------
				--"\\134aR( 100, 200 )mr"	-->	"\\134amrR( 100, 200 )"
				String = String:gsub( "(\\%d%d+[ac]^*)(%-?[%d&#]^*[%.%dH&%x]*)([km]^*[dr]*)", "%1%3%2" )
				String = String:gsub( "(\\%d%d+[ac]^*)(%b())([km]^*[dr]*)", "%1%3%2" )
				--▼--------------------------------------------------------
				String = default_time( String )
				--▼--------------------------------------------------------
				String = multi_colors( String )
				--▼--------------------------------------------------------
				String = multi_alphas( String )
				--▼--------------------------------------------------------
				String = String:gsub( "([ac]^*)%((&H%x+&)%)", "%1%2" )
				String = String:gsub( "(\\[%d]*c)([km]^*[dr]*)(%([ ]*R%b()[ ]*%))", "%1%3%2" )
				:gsub( "(\\%w+)([km]^*[dr]*)(&H%x+&)", "%1%3%2" )
				:gsub( "(\\%w+)([km]^*[dr]*)(%-?%d+[%.%d]*)", "%1%2(%3)" )
				:gsub( "(\\%w+)([km]^*[dr]*)(%b())", "%1%3%2" )
				--▼--------------------------------------------------------
				String = String:gsub( "(\\[%dv]*c)(%([ ]*R%b()[ ]*%))",
					function( Tag, Random_funct )
						local Random_funct = Random_funct:match( "R%b()" ):sub( 2, -1 )
						return format( "%s( ke4.random.color%s )", Tag, Random_funct )
					end
				) --"\\1cR( ** )" --> "\\1c .. ke4.random.color( ** )"
				--▼--------------------------------------------------------
				String = String:gsub( "\\bs(%d[fr%.%d]*)", "\\bord%1\\shad%1" )
				:gsub( "\\bss", format( "\\bord%s\\shad%s", l.outline, l.shadow ) )
				--▼--------------------------------------------------------
				-- transformación con tiempos de "ratio" o proporción (p) de cierta duración
				String = String:gsub( "\\tp([cdefisw%d%~%-]*)%((%d+[%.%d]*),(%d+[%.%d]*),",
					function( tag_mod, time_ini, time_fin )
						local time_dur = l.duration
						--[[
						if tag_mod:match( "s" ) then
							time_dur = syl.dur
						elseif tag_mod:match( "c" ) then
							time_dur = char.dur
						elseif tag_mod:match( "f" ) then
							time_dur = furi.dur
						elseif tag_mod:match( "w" ) then
							time_dur = word.dur
						end
						--]]
						local time_ini = tonumber( time_ini ) * time_dur
						local time_fin = tonumber( time_fin ) * time_dur
						return format( "\\t%s(%s,%s,", tag_mod, time_ini, time_fin )
					end
				)
				--▼--------------------------------------------------------
				-- adapta una transformación para que se ejecute desde el tiempo final
				String = String:gsub( "\\te([cfsw%~%-%d]*)(%b())",
					function( time_sign, time_tags )
						local new_tag = "\\t" .. time_sign .. time_tags
						local time_ini, time_fin, time_acc, time_mark = 0, l.duration, "", ""
						if new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
							time_ini = tonumber( new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*(%-?%d+[%.%d ]*),[ ]*%-?%d+[%.%d ]*," ) )
							time_fin = tonumber( new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*%-?%d+[%.%d ]*,[ ]*(%-?%d+[%.%d ]*)," ) )
						end
						if new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
							time_acc = new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*,[ ]*(%-?%d+[%.%d ]*,)" )
						elseif new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*%-?%d+[%.%d ]*,[ ]*\\" ) then
							time_acc = new_tag:match( "\\t[cfsw%~%-%d]*%([ ]*(%-?%d+[%.%d ]*,)[ ]*\\" )
						end
						local tags_inside = time_tags:sub( 2, -2 ):match( "\\%S+[ %S]*" )
						---------------------------------------
						local time_dur = l.duration
						--[[
						if time_sign:match( "s" ) then
							time_dur = syl.dur
						elseif time_sign:match( "c" ) then
							time_dur = char.dur
						elseif time_sign:match( "f" ) then
							time_dur = furi.dur
						elseif time_sign:match( "w" ) then
							time_dur = word.dur
						end
						--]]
						---------------------------------------
						local time_end_ini = time_dur - time_fin
						local time_end_fin = time_dur - time_ini
						--[[
						local transfo_esp = {
							[ 01 ] = "1",	[ 02 ] = "2",	[ 03 ] = "3",	[ 04 ] = "4",
							[ 05 ] = "5",	[ 06 ] = "6",	[ 07 ] = "7",	[ 08 ] = "8",
							[ 09 ] = "9",	[ 10 ] = "0",	[ 11 ] = "~",	[ 12 ] = "-",
						}
						if table.inside( transfo_esp, time_sign ) then
							time_mark = "M"
						end --no recuerdo para qué es esta marca :P
						--]]
						return format( "\\t%s(%s,%s,%s)%s", time_sign, time_end_ini, time_end_fin, time_acc .. tags_inside, time_mark )
					end --"\\te(0,300,\\frR( 45 ))" = \\t(fx.dur-300,fx.dur,\\frR( 45 ))
				) --"\\te(0,100,\\frR( 45 ):0)"
				--▼--------------------------------------------------------
				local tag_fun = {
					-- tags funciones: todos aquellos que están seguidos de paréntesis
					[ 001 ] = "\\1vc",		[ 002 ] = "\\2vc",		[ 003 ] = "\\3vc",		[ 004 ] = "\\4vc",
					[ 005 ] = "\\1va",		[ 006 ] = "\\2va",		[ 007 ] = "\\3va",		[ 008 ] = "\\4va",
					[ 009 ] = "\\1img",		[ 010 ] = "\\2img",		[ 011 ] = "\\3img",		[ 012 ] = "\\4img",
					[ 013 ] = "\\pos",		[ 014 ] = "\\move",		[ 015 ] = "\\moves3",	[ 016 ] = "\\moves4",
					[ 017 ] = "\\mover",	[ 018 ] = "\\movevc",	[ 019 ] = "\\org",		[ 020 ] = "\\disTorT",
					[ 021 ] = "\\fad",		[ 022 ] = "\\fade",		[ 023 ] = "\\jiTTer",	[ 024 ] = "\\clip",
					[ 025 ] = "\\iclip",	[ 026 ] = "\\t",		[ 027 ] = "\\t~",		[ 028 ] = "\\t-",
					[ 029 ] = "\\tc",		[ 030 ] = "\\td",		[ 031 ] = "\\tf",		[ 032 ] = "\\ti",
					[ 033 ] = "\\ts",		[ 034 ] = "\\tw",		[ 035 ] = "\\t0",		[ 036 ] = "\\t1",
					[ 037 ] = "\\t2",		[ 038 ] = "\\t3",		[ 039 ] = "\\t4",		[ 040 ] = "\\t5",
					[ 041 ] = "\\t6",		[ 042 ] = "\\t7",		[ 043 ] = "\\t8",		[ 044 ] = "\\t9",
					[ 045 ] = "\\tc~",		[ 046 ] = "\\tf~",		[ 047 ] = "\\ts~",		[ 048 ] = "\\tw~",
					[ 049 ] = "\\tc-",		[ 050 ] = "\\tf-",		[ 051 ] = "\\ts-",		[ 052 ] = "\\tw-",
					[ 053 ] = "\\tc0",		[ 054 ] = "\\tf0",		[ 055 ] = "\\ts0",		[ 056 ] = "\\tw0",
					[ 057 ] = "\\tc1",		[ 058 ] = "\\tf1",		[ 059 ] = "\\ts1",		[ 060 ] = "\\tw1",
					[ 061 ] = "\\tc2",		[ 062 ] = "\\tf2",		[ 063 ] = "\\ts2",		[ 064 ] = "\\tw2",
					[ 065 ] = "\\tc3",		[ 066 ] = "\\tf3",		[ 067 ] = "\\ts3",		[ 068 ] = "\\tw3",
					[ 069 ] = "\\tc4",		[ 070 ] = "\\tf4",		[ 071 ] = "\\ts4",		[ 072 ] = "\\tw4",
					[ 073 ] = "\\tc5",		[ 074 ] = "\\tf5",		[ 075 ] = "\\ts5",		[ 076 ] = "\\tw5",
					[ 077 ] = "\\tc6",		[ 078 ] = "\\tf6",		[ 079 ] = "\\ts6",		[ 080 ] = "\\tw6",
					[ 081 ] = "\\tc7",		[ 082 ] = "\\tf7",		[ 083 ] = "\\ts7",		[ 084 ] = "\\tw7",
					[ 085 ] = "\\tc8",		[ 086 ] = "\\tf8",		[ 087 ] = "\\ts8",		[ 088 ] = "\\tw8",
					[ 089 ] = "\\tc9",		[ 090 ] = "\\tf9",		[ 091 ] = "\\ts9",		[ 092 ] = "\\tw9",
					[ 093 ] = "\\tp",		[ 094 ] = "\\tsx",		[ 095 ] = "\\tcx",		[ 986 ] = "\\tfx",
					[ 097 ] = "\\twx",
				}
				local tags_out = {
					-- los tags que quedan excluidos de ser remplazados en esta función, ya que de otro
					-- modo,  el valor random sería el mismo para cada uno de los tags que los componen
					[ 001 ] = "\\fscxy(R",	[ 002 ] = "\\frxy(R",	[ 003 ] = "\\frxz(R",	[ 004 ] = "\\fryz(R",
					[ 005 ] = "\\frxyz(R",	[ 006 ] = "\\faxy(R",	[ 007 ] = "\\fscxyr(R",	[ 008 ] = "\\frxyo(R",
					[ 009 ] = "\\frxzo(R",	[ 010 ] = "\\fryzo(R",	[ 011 ] = "\\frxyzo(R",	[ 012 ] = "\\xybord(R",
					[ 013 ] = "\\xyshad(R",
				}
				--▼--------------------------------------------------------
				String = String:gsub( "\\%w+%b()", -- tags seguidos de paréntesis :)
					function( capture )
						-------------------------------------------------------------------
						--local line = linefx[ ii ]
						local tag_capture = ke4.tag.operation( capture:match( "(\\%w+)%b()" ) )
						local val_capture = capture:match( "\\%w+(%b())" ):sub( 2, -2 )
						-------------------------------------------------------------------
						if ke4.table.inside( tags_out, capture:match( "\\%w+%(R" ) ) == false then
							if type( ke4.table.index( tag_fun, tag_capture ) ) == "number" then
								--[ 1 ]: transformaciones
								if ke4.table.index( tag_fun, tag_capture ) >= 26 then
									val_capture = val_capture:gsub( "\\%w+%b()",
										function( time_capture )
											--------------------------------------------------------------------------
											--local line = linefx[ ii ]
											local t_tag_capture = ke4.tag.operation( time_capture:match( "(\\%w+)%b()" ) )
											local t_val_capture = time_capture:match( "\\%w+(%b())" ):sub( 2, -2 )
											--------------------------------------------------------------------------
											if ke4.table.inside( tags_out, time_capture:match( "\\%w+%(R" ) ) == false then
												--[ 2 ]: tags funciones dentro de una transformación
												if type( ke4.table.index( tag_fun, t_tag_capture ) ) == "number" then
													if ke4.table.index( tag_fun, t_tag_capture ) >= 13
														and ke4.table.index( tag_fun, t_tag_capture ) <= 25 then
														if type( ke4.string.toval( "{" .. t_val_capture .. "}" ) ) == "table" then
															return format( "%s(%s)", t_tag_capture, ke4.table.op( ke4.string.toval( "{" .. t_val_capture .. "}" ), "concat", "," ) )
														end --"\\t(\\clip( line.left, line.top, line.right, line.bottom ))"
														return format( "%s(%s)", t_tag_capture, t_val_capture )
													end
												end
												--[ 3 ]: tags funciones del 1 al 12 dentro de una transformación
												if ke4.table.inside( tag_fun, t_tag_capture ) then -- tags del MOD
													return time_capture
												end
												--[ 4 ]: tags dentro de una transformación
												return t_tag_capture .. ke4.string.toval( t_val_capture )
												--"\\t(\\blur( 3 + demo ))"
											end
											return time_capture
										end
									)
									return format( "%s(%s)", tag_capture, val_capture )
								end --"\\t(0,1000,\\blurR( 8 ))"
								--[ 5 ]: tags de posición, \\org y otros tags funciones
								if ke4.table.index( tag_fun, tag_capture ) >= 13
									and ke4.table.index( tag_fun, tag_capture ) <= 25 then
									if type( ke4.string.toval( "{" .. val_capture .. "}" ) ) == "table" then
										return format( "%s(%s)", tag_capture, ke4.table.op( ke4.string.toval( "{" .. val_capture .. "}" ), "concat", "," ) )
									end --"\\org( line.left, line.middle )"
									return capture
								end
								return capture
							end
							--[ 6 ]: tags que no son funciones
							return tag_capture .. ke4.string.toval( val_capture )
							--"\\blur( 3 + 5m )"
						end
						return capture
					end
				)
				--▼--------------------------------------------------------
				-- transformaciones de movimientos de clip's
				String = String:gsub( "(\\i?clip)(%b())[ ]*(%b())",
					function( Tag, Coor, Times_moves )
						local coors = { }
						local clip_moves = ""
						for num in Coor:gmatch( "%-?%d+[%.%d]*" ) do
							table.insert( coors, tonumber( num ) )
						end
						if #coors >= 4
							and Coor:match( "m" ) == nil then
							local tmoves = { }
							local t_moves = Times_moves:sub( 2, -2 )
							if pcall( loadstring( format( "return function( meta, line, x, y ) return { %s } end", t_moves ) ) ) then
								local tmoves_func = loadstring( format( "return function( meta, line, x, y ) return { %s } end", t_moves ) )( )
								if pcall( tmoves_func ) then
									tmoves = tmoves_func( meta, line, x, y )
								end
								for i = 1, #tmoves do
									clip_moves = clip_moves .. format( "\\t(%s,%s,%s,%s(%s,%s,%s,%s))",
										tmoves[ i ][ 1 ], tmoves[ i ][ 2 ], tmoves[ i ][ 5 ] or 1, Tag,
										coors[ 1 ] + tmoves[ i ][ 3 ], coors[ 2 ] + tmoves[ i ][ 4 ],
										coors[ 3 ] + tmoves[ i ][ 3 ], coors[ 4 ] + tmoves[ i ][ 4 ]
									)
								end
								clip_moves = clip_moves:gsub( "(\\t%(%d+[%.%d]*%,%d+[%.%d]*%,)1%,", "%1" )
								return Tag .. Coor .. clip_moves
							end
						end                                          --   t1  t1   x     y	  accel*
					end --"\\clip(fx.pos_l,fx.pos_t,fx.pos_r,fx.pos_b)( { 0, 200, 120, -342, [ 0.8 ] } )
				)
				return String
			end,

			dark = function( Text_Config, String )
				local l = ke4.table.duplicate( Text_Config.styleref )
				l.duration, l.center, l.middle = Text_Config.duration, Text_Config.center, Text_Config.middle
				local text_color1, text_color2 = ke4.color.fromstyle( l.color1 ), ke4.color.fromstyle( l.color2 )
				local text_color3, text_color4 = ke4.color.fromstyle( l.color3 ), ke4.color.fromstyle( l.color4 )
				local text_alpha1, text_alpha2 = ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color2 )
				local text_alpha3, text_alpha4 = ke4.alpha.fromstyle( l.color3 ), ke4.alpha.fromstyle( l.color4 )
				String = ke4.tag.redefine( Text_Config, String )
				------------------------------------------------------------------------------------------
				local tags_i = {
					[ 01 ] = "(\\fscxy)",			[ 02 ] = "(\\frxy)",			[ 03 ] = "(\\frxz)",
					[ 04 ] = "(\\fryz)",			[ 05 ] = "(\\frxyz)",			[ 06 ] = "(\\faxy)",
					[ 07 ] = "(\\fscxr)",			[ 08 ] = "(\\fscyr)",			[ 09 ] = "(\\fscxyi)",
					[ 10 ] = "(\\fscxy[i]*r)",		[ 11 ] = "(\\frxo)",			[ 12 ] = "(\\fryo)",
					[ 13 ] = "(\\fr[z]*o)",			[ 14 ] = "(\\frxyo)",			[ 15 ] = "(\\frxzo)",
					[ 16 ] = "(\\fryzo)",			[ 17 ] = "(\\frxyzo)",			[ 18 ] = "(\\xybord)",
					[ 19 ] = "(\\xyshad)",
				}
				------------------------------------------------------------------------------------------
				local tags_v = {
					[ 01 ] = "(%-?%d+[%.%d]*)",
					[ 02 ] = "(R[cdemrs]*%b())",
					[ 03 ] = "(%b())",
					[ 04 ] = "([%-%~kmdr]*)",
				}
				------------------------------------------------------------------------------------------
				local tags_r = {
					[ 01 ] = "\\fscx%s\\fscy%s",					[ 02 ] = "\\frx%s\\fry%s",
					[ 03 ] = "\\frx%s\\frz%s",						[ 04 ] = "\\fry%s\\frz%s",
					[ 05 ] = "\\frx%s\\fry%s\\frz%s",				[ 06 ] = "\\fax%s\\fay%s",
					[ 07 ] = "\\fscxr%s",							[ 08 ] = "\\fscyr%s",
					[ 09 ] = "\\fscx%s\\fscy%s",					[ 10 ] = "\\fscxr%s\\fscyr%s",
					[ 11 ] = "\\org(%s,%s)\\frx%s",					[ 12 ] = "\\org(%s,%s)\\fry%s",
					[ 13 ] = "\\org(%s,%s)\\frz%s",					[ 14 ] = "\\org(%s,%s)\\frx%s\\fry%s",
					[ 15 ] = "\\org(%s,%s)\\frx%s\\frz%s",			[ 16 ] = "\\org(%s,%s)\\fry%s\\frz%s",
					[ 17 ] = "\\org(%s,%s)\\frx%s\\fry%s\\frz%s",	[ 18 ] = "\\xbord%s\\ybord%s",
					[ 19 ] = "\\xshad%s\\yshad%s",
				}
				------------------------------------------------------------------------------------------
				String = String:gsub( "%.line", ".LINE" ) --> var.line.val
				:gsub( "meta%.res_x", "xres" ):gsub( "meta%.res_y", "yres" )
				:gsub( "line%.i", "l_counter" ):gsub( "line%.n", "maxil_counter" )
				:gsub( "line%.", "linefx[  ii  ]." )
				:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
				:gsub( "%.LINE", ".line" )
				------------------------------------------------------------------------------------------
				local v01, v02
				ext_funct_b, ext_funct_v = nil, ""
				for i = 1, #tags_i do
					for k = 1, 3 do
						----------------------------------------------------------------------------------------------
						if String:match( tags_i[ i ] .. tags_v[ k ] .. "([%~%-]^*%d+[%.%d]*)" ) then
							ext_funct_b = true
							v01, v02, ext_funct_v = String:match( tags_i[ i ] .. tags_v[ k ] .. "([%~%-]^*%d+[%.%d]*)" )
							String = String:gsub( tags_i[ i ] .. tags_v[ k ] .. "([%~%-]^*%d+[%.%d]*)", "%1%2" )
						elseif String:match( tags_i[ i ] .. tags_v[ k ] .. "([%~%-]^*%b())" ) then
							ext_funct_b = true
							v01, v02, ext_funct_v = String:match( tags_i[ i ] .. tags_v[ k ] .. "([%~%-]^*%b())" )
							String = String:gsub( tags_i[ i ] .. tags_v[ k ] .. "([%~%-]^*%b())", "%1%2" )
						end --"\\xyshad-6r~300"
						----------------------------------------------------------------------------------------------
						String = String:gsub( tags_i[ i ] .. tags_v[ k ] .. tags_v[ 4 ],
							function( tagdark, capture, add_function )
								local extra_function = add_function
								if ext_funct_b then
									extra_function = ext_funct_v
									ext_funct_b = nil
								end
								--local line = linefx[ ii ]
								if pcall( loadstring( format( "return function( meta, line, x, y ) return %s end", capture ) ) ) then
									local fun_dark = loadstring( format( "return function( meta, line, x, y ) return %s end", capture ) )( )
									if pcall( fun_dark ) then
										if tags_i[ i ]:match( "\\fscxyi" )
											and fun_dark( meta, line, x, y ) then
											local size_xy = fun_dark( meta, line, x, y )
											if size_xy then
												return format( "@" .. tags_r[ i ] .. "@",
													size_xy .. extra_function,
													size_xy .. extra_function
												)
											end
										end --"\\fscxyiRm( 100, 200 )"
										if tags_r[ i ]:match( "\\org" )
											and fun_dark( meta, line, x, y ) then
											return format( "@" .. tags_r[ i ] .. "@",
												l.center, l.middle,
												fun_dark( meta, line, x, y ) .. extra_function,
												fun_dark( meta, line, x, y ) .. extra_function,
												fun_dark( meta, line, x, y ) .. extra_function
											)
										end --"\\t(100,800,\\frxyoR( 20, 77 ))"
										if fun_dark( meta, line, x, y ) then
											return format( "@" .. tags_r[ i ] .. "@",
												fun_dark( meta, line, x, y ) .. extra_function,
												fun_dark( meta, line, x, y ) .. extra_function,
												fun_dark( meta, line, x, y ) .. extra_function
											) --"\\frxyzRds( 360 )"
										end
									end
								end
								return tagdark .. capture .. extra_function
							end
						)
					end
				end
				---------------------------------------------------------------------
				String = String:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
				:gsub( "l_counter", "line.i" ):gsub( "maxil_counter", "line.n")
				:gsub( "linefx%[  ii  %]%.", "line." )
				:gsub( "\"(&H%x+&)\"", "%1" )
				--"\\t(100,800,\\frxyoR( 10 * line.i, 77 ))"
				-- convierte los tags de escala, de ratio a porcentaje
				String = String:gsub( "(\\fsc[xy]^*)r(%-?%d+[%.%d]*)",
					function( tag_fsc, ratioxy )
						local Scale = l.scale_x
						if tag_fsc:match( "y" ) then
							Scale = l.scale_y
						end
						return tag_fsc .. Scale * tonumber( ratioxy )
					end --"\\fscxyr1.4"
				)
				--convierte el tag \\fs de ratio a pixeles
				String = String:gsub( "\\fsr(%-?%d+[%.%d]*)",
					function( ratiofs )
						return "\\fs" .. l.fontsize * tonumber( ratiofs )
					end
				)
				-- tiempo de la transformación por default o del valor inverso
				String = String:gsub( "([%~%-]^*)(%b())",
					function( tag_sig, tag_val )
						return tag_sig .. ke4.string.toval( tag_val )
					end -- "\\xshad-8r~300" = "\\xshad-8r\\t(0,300,\\xshad( l.shadow ))"
				) -- "\\yshad10r-(fx.dur - 200)" = "\\yshad10r\\t(0,fx.dur - 200,\\yshad-10r)"
				-----------------------------------------------------------------------------------------
				-- le da el valor por default o inverso a un tag y lo pone dentro de una transformación
				String = String:gsub( "(\\[%d]*%a+%-?[%d&]^*[%d%.&H%x]*)([%-%~]^*)(%-?%d+[%.%d]*)",
					function( tag_cap, mark_sign, time_transfo )
						local def_or_inv = ke4.tag.default
						if mark_sign == "-" then
							def_or_inv = ke4.tag.inverse
						end
						return format( "%s\\t(0,%s,%s)", tag_cap, time_transfo, def_or_inv( Text_Config, tag_cap ) )
					end --"\\xshad( -9 )-440"
				) --tag.oscill( fx.dur, 1000, "\\fr( 5i )\\frx( 3 - i )" )
				String = String:gsub( "(\\[%d]*%a+%-?[%d&]^*[%d%.&H%x]*)([%-%~]^*)",
					function( tag_cap, mark_sign )
						local def_or_inv = ke4.tag.default
						if mark_sign == "-" then
							def_or_inv = ke4.tag.inverse
						end
						return format( "%s\\t(%s)", tag_cap, def_or_inv( Text_Config, tag_cap ) )
					end --"\\xshad-8r~"
				) --"\\yshad10r-200"
				--------------------------------------------------------------------------
				-- reune transformaciones creadas por un tag dark, en una sola
				String = String:gsub( "%b@@",	
					function( capture )
						local time_capture_def = { }
						local time_capture_val = { }
						capture = capture:gsub( "\\t[cdfisw%d]*%b()",
							function( capture2 )
								if capture2:match( "\\t[cdfisw%d]*%(\\" ) then
									table.insert( time_capture_def, capture2:match( "%b()" ):sub( 2, -2 ) )
								else
									table.insert(
										time_capture_val,
										capture2:match( "\\t[cdfisw%d]*(%b())" ):sub( 2, -2 ):match( "\\%S+[ %S]*" )
									)
								end --"\\frxyzo-8-1200"
								capture2 = capture2 .. "def"
								return capture2
							end
						)
						if #time_capture_def > 1 then
							local tag_time = capture:match( "(\\t[cdfisw%d]*)%b()" )
							capture = capture:gsub( "\\t[cdfisw%d]*%b()def", "", #time_capture_def - 1 )
							capture = capture:gsub( "\\t[cdfisw%d]*%b()def",
								function( capture3 )
									return format( "%s(%s)", tag_time, ke4.table.op( time_capture_def, "concat" ) )
								end
							)
						end
						if #time_capture_val > 1 then
							capture = capture:gsub( "\\t[cdfisw%d]*%b()def", "", #time_capture_val - 1 )
							capture = capture:gsub( "(\\t[cdfisw%d]*%b())def",
								function( capture3 )
									capture3 = capture3:match( "\\t[cdfisw%d]*%(%d+[%.%d]*,%d+[%.%d]*,[%.%d%,]*" )
									capture3 = capture3 .. ke4.table.op( time_capture_val, "concat" ) .. ")"
									return capture3
								end
							)
						end --"\\xyshadRds( 12 )-300"
						capture = capture:gsub( "(\\t[cdfisw%d]*%b())(def)", "%1" ):gsub( "@", "" )
						return capture
					end --"\\frxyzRds( 360 )~800"
				)
				--------------------------------------------------------------------------
				-- le da el valor por default o inverso a un grupo de tags y los pone dentro de una transformación
				String = String:gsub( "(%b())([%-%~]^*)(%-?%d+[%.%d]*)",
					function( tag_cap, mark_sign, time_transfo )
						local tag_cap = tag_cap:sub( 2, -2 )
						local def_or_inv = ke4.tag.default
						if tag_cap:match( "\\%w+%-?[%d&]^*[%d%.&H%x]*" ) then
							if mark_sign == "-" then
								def_or_inv = ke4.tag.inverse
							end
							return format( "%s\\t(0,%s,%s)", tag_cap, time_transfo, def_or_inv( Text_Config, tag_cap ) )
						end
					end --"\\bord5r(\\xyshad-4\\fr45)-360"
				)
				String = String:gsub( "(%b())([%-%~]^*)",
					function( tag_cap, mark_sign )
						local tag_cap = tag_cap:sub( 2, -2 )
						local def_or_inv = ke4.tag.default
						if tag_cap:match( "\\%w+%-?[%d&]^*[%d%.&H%x]*" ) then
							if mark_sign == "-" then
								def_or_inv = ke4.tag.inverse
							end
							return format( "%s\\t(%s)", tag_cap, def_or_inv( Text_Config, tag_cap ) )
						end --"(\\xyshad-5)~"
					end --"(\\frx300\\blur4\\3c&HFFFFFF&)~"
				)
				String = String:gsub( "(\\%w+%-?[%d&#]^*[%.%dH&%x]*) [	 ]*(\\)", "%1%2" )
				-----------------------------------------------------------------------------------------
				-- genera dos o tres transformaciones con duración promedia
				-- del efecto con valores inversos o por default, de un tag
				String = String:gsub( "(\\[%d]*%a+)(%-?[%d&]^*[%.%dH%x&]*)([km]^*[dr]*)",
					function( tag_num, time_val, time_dec )
						local l = Text_Config
						local val_nor = tag_num .. time_val
						local val_def = ke4.tag.default( Text_Config, val_nor )
						local val_inv = ke4.tag.inverse( Text_Config, val_nor )
						if time_dec == "mr" then
							-- normal + inverso + normal
							return format( "%s\\t(%s,%s,%s)tmid1\\t(%s,%s,%s)tmid2",
								val_nor,
								0.00 * l.duration, 0.50 * l.duration, val_inv,
								0.50 * l.duration, 1.00 * l.duration, val_nor
							) --"\\frxzomrR( 45 )"
						elseif time_dec == "md" then
							-- normal + default + normal
							return format( "%s\\t(%s,%s,%s)tmid1\\t(%s,%s,%s)tmid2",
								val_nor,
								0.00 * l.duration, 0.50 * l.duration, val_def,
								0.50 * l.duration, 1.00 * l.duration, val_nor
							)
						elseif time_dec == "mrd"
							or time_dec == "mdr" then
							-- normal + inverso + default
							return format( "%s\\t(%s,%s,%s)tmid1\\t(%s,%s,%s)tmid2",
								val_nor,
								0.00 * l.duration, 0.50 * l.duration, val_inv,
								0.50 * l.duration, 1.00 * l.duration, val_def
							)
						elseif time_dec == "k" then
							-- normal + inverso + default
							return format( "\\t(%s,%s,%s)tter1\\t(%s,%s,%s)tter2\\t(%s,%s,%s)tter3",
								0.00 * l.duration, 0.25 * l.duration, val_nor,
								0.25 * l.duration, 0.75 * l.duration, val_inv,
								0.75 * l.duration, 1.00 * l.duration, val_def
							)
						elseif time_dec == "kr" then
							-- normal + inverso + normal
							return format( "\\t(%s,%s,%s)tter1\\t(%s,%s,%s)tter2\\t(%s,%s,%s)tter3",
								0.00 * l.duration, 0.25 * l.duration, val_nor,
								0.25 * l.duration, 0.75 * l.duration, val_inv,
								0.75 * l.duration, 1.00 * l.duration, val_nor
							)
						elseif time_dec == "kd" then
							-- normal + default + normal
							return format( "\\t(%s,%s,%s)tter1\\t(%s,%s,%s)tter2\\t(%s,%s,%s)tter3",
								0.00 * l.duration, 0.33 * l.duration, val_nor,
								0.33 * l.duration, 0.66 * l.duration, val_def,
								0.66 * l.duration, 1.00 * l.duration, val_nor
							)
						end -- sobran: kdr, krd, m
					end --"\\13cmrR( )"
				) --"\\frxzmrR(211)"
				-- unifica las transformaciones creadas por las letras [mk]^*[rd]*
				local function time_cap_uni( String )
					times_from_dark = {
						[ 1 ] = "(\\t[cdfisw%d]*%b())(tmid1)",
						[ 2 ] = "(\\t[cdfisw%d]*%b())(tmid2)",
						[ 3 ] = "(\\t[cdfisw%d]*%b())(tter1)",
						[ 4 ] = "(\\t[cdfisw%d]*%b())(tter2)",
						[ 5 ] = "(\\t[cdfisw%d]*%b())(tter3)",
					}
					for i = 1, #times_from_dark do
						local time_capture_uni = { }
						String = String:gsub( times_from_dark[ i ],
							function( capture, time_type )
								table.insert(
									time_capture_uni,
									capture:match( "\\t[cdfisw%d]*(%b())" ):sub( 2, -2 ):match( "\\%S+[ %S]*" )
								)
								capture = capture .. "def"
								return capture
							end
						)
						if #time_capture_uni > 1 then
							String = String:gsub( "\\t[cdfisw%d]*%b()def", "", #time_capture_uni - 1 )
							String = String:gsub( "(\\t[cdfisw%d]*%b())def",
								function( capture2 )
									capture2 = capture2:match( "\\t[cdfisw%d]*%(%d+[%.%d]*,%d+[%.%d]*,[%.%d%,]*" )
									capture2 = capture2 .. ke4.table.op( time_capture_uni, "concat" ) .. ")"
									return capture2
								end
							)
						end
						String = String:gsub( "(\\t[cdfisw%d]*%b())(def)", "%1" )
					end --"\\frxymrRs( 86 )\\xyshadmrRrs( 2, 10 )"
					return String
				end
				String = time_cap_uni( String )
				-- nuevas transformaciones con valores inversos y/o por defaults
				String = String:gsub( "(\\t[cfsw]*)([%-%~%d]^*)(%b())",
					function( tag_transfo, indicator, time_valors )
						local l = Text_Config
						local new_tag = tag_transfo .. indicator .. time_valors
						local time_ini, time_fin = 0, l.duration
						--[[
						if tag_transfo:match( "ts" ) then
							time_fin = syl.dur
						elseif tag_transfo:match( "tc" ) then
							time_fin = char.dur
						elseif tag_transfo:match( "tf" ) then
							time_fin = furi.dur
						elseif tag_transfo:match( "tw" ) then
							time_fin = word.dur
						end
						--]]
						if new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*," ) then
							time_ini = tonumber( new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*(%d+[%.%d ]*),[ ]*%d+[%.%d ]*," ) )
							time_fin = tonumber( new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*%d+[%.%d ]*,[ ]*(%d+[%.%d ]*)," ) )
						end
						local time_acc = ""
						if new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*," ) then
							time_acc = new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*,[ ]*(%d+[%.%d ]*,)" )
						elseif new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*%d+[%.%d ]*,[ ]*\\" ) then
							time_acc = new_tag:match( "\\t[cfsw]*[%-%~%d]^*%([ ]*(%d+[%.%d ]*,)[ ]*\\" )
						end
						local time_dur = time_fin - time_ini
						local tag_time = tag_transfo:match( "\\(t[cfsw]*)" )
						local valors_def = ke4.tag.default( Text_Config, time_valors )
						local valors_inv = ke4.tag.inverse( Text_Config, time_valors )
						local valors_nor = time_valors:sub( 2, -2 ):match( "\\%S+[ %S]*" ) --> captura tags dentro de los \\t
						if indicator == "~" then
							-- normal + default
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.00 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.00 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_def
							) -- "\\t~(\\xshad-4)"
						elseif indicator == "-" then
							-- normal + inverso
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.00 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.00 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_inv
							) -- "\\t-(\\fscxyr1.5)"
						elseif indicator == "1" then
							-- normal + inverso + normal
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.00 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.50 * time_dur, time_acc .. valors_inv,
								tag_time, time_ini + 0.50 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_nor
							) --]]-- "\\t1(1000,2000,\\xshad-4\\fscxyr1.5)"
						elseif indicator == "2" then
							-- normal + default + normal
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.00 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.50 * time_dur, time_acc .. valors_def,
								tag_time, time_ini + 0.50 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_nor
							) -- "\\t2(1000,2000,\\xshad-4\\fscxyr1.5)"
						elseif indicator == "3" then
							-- normal + inverso + default
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.00 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.50 * time_dur, time_acc .. valors_inv,
								tag_time, time_ini + 0.50 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_def
							) -- "\\t3(\\frxyzRs( 20, 60 ))"
						elseif indicator == "4" then
							-- normal + inverso + default
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.25 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.25 * time_dur, time_ini + 0.75 * time_dur, time_acc .. valors_inv,
								tag_time, time_ini + 0.75 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_def
							) -- "\\t4(\\xshad-10r\\1cR( ))"
						elseif indicator == "5" then
							-- normal + default
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.50 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.50 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_def
							) -- "\\t5(0,320,\\fscxyr1.45\\blur1.4)"
						elseif indicator == "6" then
							-- normal + inverse
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini + 0.00 * time_dur, time_ini + 0.50 * time_dur, time_acc .. valors_nor,
								tag_time, time_ini + 0.50 * time_dur, time_ini + 1.00 * time_dur, time_acc .. valors_inv
							) -- "\\t6(0,320,\\frxzRs( 60, 100 ))"
						elseif indicator == "7" then
							-- normal + defautl
							if time_dur == l.duration then
								return format( "\\%s(%s,%s,%s)", tag_time, time_ini, time_fin, time_acc .. valors_nor )
							end
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini, time_fin, time_acc .. valors_nor,
								tag_time, time_fin, l.duration, time_acc .. valors_def
							) -- "\\t7(0,320,\\frxzRs( 60, 100 ))"
						elseif indicator == "8" then
							-- normal + inverse
							if time_dur == l.duration then
								return format( "\\%s(%s,%s,%s)", tag_time, time_ini, time_fin, time_acc .. valors_nor )
							end
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini, time_fin, time_acc .. valors_nor,
								tag_time, time_fin, l.duration, time_acc .. valors_inv
							) -- "\\t8(0,320,\\frxzRs( 60, 100 ))"
						elseif indicator == "9" then
							-- normal + default + normal
							if time_dur >= l.duration then
								return format( "\\%s(%s,%s,%s)", tag_time, time_ini, time_fin, time_acc .. valors_nor )
							end
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini, time_ini, valors_nor,
								tag_time, time_ini, time_fin, time_acc .. valors_def,
								tag_time, l.duration - time_fin, l.duration - time_ini, time_acc .. valors_nor
							) -- "\\t9(0,320,\\frxzRs( 60, 100 ))"
						elseif indicator == "0" then
							-- normal + default + inverse
							if time_dur >= l.duration then
								return format( "\\%s(%s,%s,%s)", tag_time, time_ini, time_fin, time_acc .. valors_nor )
							end
							return format( "\\%s(%s,%s,%s)\\%s(%s,%s,%s)\\%s(%s,%s,%s)",
								tag_time, time_ini, time_ini, valors_nor,
								tag_time, time_ini, time_fin, time_acc .. valors_def,
								tag_time, l.duration - time_fin, l.duration - time_ini, time_acc .. valors_inv
							) -- "\\t0(0,320,\\frxzRs( 60, 100 ))"
						end
					end
				)
				-------------------------------------
				String = ke4.tag.timefx( Text_Config, String )
				-------------------------------------
				String = String:gsub( "\\org%([ ]*%)",
					function( org_cap )
						return format( "\\org(%s,%s)", l.center, l.middle )
					end
				)
				-------------------------------------
				-- extrae el tag \\org que esté dentro de una transformación
				String = String:gsub( "\\t[cdefiswx%d%~%-]*%b()",
					function( time_cap )
						if time_cap:match( "\\org%b()" ) then
							local org_tag = time_cap:match( "\\org%b()" )
							time_cap = org_tag .. time_cap:gsub( "\\org%b()", "" )
						end
						time_cap = time_cap:gsub( "\\t[di]^*%(", "\\t%(" )
						return time_cap
					end
				)
				-------------------------------------
				-- interpola, en el caso de "recorte", los tags dentro de las transfos
				String = String:gsub( "(\\t[%w%~%-]*)(%b())",
					function( tag_time, capture )
						local l = Text_Config.styleref
						l.duration, l.center, l.middle = Text_Config.duration, Text_Config.center, Text_Config.middle
						local text_color1, text_color2 = ke4.color.fromstyle( l.color1 ), ke4.color.fromstyle( l.color2 )
						local text_color3, text_color4 = ke4.color.fromstyle( l.color3 ), ke4.color.fromstyle( l.color4 )
						local text_alpha1, text_alpha2 = ke4.alpha.fromstyle( l.color1 ), ke4.alpha.fromstyle( l.color2 )
						local text_alpha3, text_alpha4 = ke4.alpha.fromstyle( l.color3 ), ke4.alpha.fromstyle( l.color4 )
						local tcapture = tag_time .. capture
						local time_ini, time_fin, time_acc, time_dur, ipol_ini, ipol_fin
						local tags_in_t, tags_table = "", { }
						if tcapture:match( "\\t[%w%~%-]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
							if capture:match( "\\%w+[ %S]*" ) then
								tags_in_t = capture:match( "\\%w+[ %S]*" ):sub( 1, -2 )
							end
							for tags in tags_in_t:gmatch( "\\[%d]*%a+%-?[%d&]^*[%.%dH&%#%x%(%)]*" ) do
								table.insert( tags_table, tags )
							end
							if #tags_table > 0 then
								time_ini, time_fin = tcapture:match( "\\t[%w%~%-]*%([ ]*(%-?%d+[%.%d ]*),[ ]*(%-?%d+[%.%d ]*)," )
								time_ini, time_fin = ke4.math.round( time_ini, 1 ), ke4.math.round( time_fin, 1 )
								time_dur = time_fin - time_ini
								time_acc = 1
								if tcapture:match( "\\t[%w%~%-]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
									time_acc = tcapture:match( "\\t[%w%~%-]*%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*,[ ]*(%-?%d+[%.%d ]*)," )
								end
								ipol_ini = abs( time_ini ) / time_dur
								ipol_fin = ( -1 * time_ini + ke4.math.round( l.duration, 1 )) / time_dur
								local tags_default_tags = {
									[ 01 ] = "\\1v?c",			[ 02 ] = "\\2v?c",			[ 03 ] = "\\3v?c",
									[ 04 ] = "\\4v?c",			[ 05 ] = "(\\c)&H",			[ 06 ] = "\\1v?a",
									[ 07 ] = "\\2v?a",			[ 08 ] = "\\3v?a",			[ 09 ] = "\\4v?a",
									[ 10 ] = "\\alpha",			[ 11 ] = "\\fsp",			[ 12 ] = "\\fsvp",
									[ 13 ] = "\\blur",			[ 14 ] = "\\fe",			[ 15 ] = "\\be",
									[ 16 ] = "\\rnd[xyz]*",		[ 17 ] = "\\frs",			[ 18 ] = "\\[xy]*bord",
									[ 19 ] = "\\fn",			[ 20 ] = "\\z",				[ 21 ] = "\\[xy]*shad",
									[ 22 ] = "\\fsc",			[ 23 ] = "\\fs",			[ 24 ] = "\\fa[xy]^*",
									[ 25 ] = "\\fr[xy]^*",		[ 26 ] = "\\fr[z]*",		[ 27 ] = "\\fscx",
									[ 28 ] = "\\fscy",
								}
								local tags_deafult_vals = {
									[ 01 ] = text_color1,		[ 02 ] = text_color2,		[ 03 ] = text_color3,
									[ 04 ] = text_color4,		[ 05 ] = text_color1,		[ 06 ] = text_alpha1,
									[ 07 ] = text_alpha2,		[ 08 ] = text_alpha3,		[ 09 ] = text_alpha4,
									[ 10 ] = "&H00&",			[ 11 ] = l.spacing,			[ 12 ] = 0,
									[ 13 ] = 0,					[ 14 ] = 0,					[ 15 ] = 0,
									[ 16 ] = 0,					[ 17 ] = 0,					[ 18 ] = l.outline,
									[ 19 ] = l.fontname,		[ 20 ] = 0,					[ 21 ] = l.shadow,
									[ 22 ] = l.scale_x,			[ 23 ] = l.fontsize,		[ 24 ] = 0,
									[ 25 ] = 0,					[ 26 ] = l.angle,			[ 27 ] = l.scale_x,
									[ 28 ] = l.scale_y,
								}
								local tags_ipol_ini, tags_ipol_fin = "", ""
								local tag_idx, tag_nam, tag_val = 0
								for i = 1, #tags_table do
									tag_nam, tag_val = tags_table[ i ]:match( "(\\[%d]*%a+)(%-?[%d&]^*[%.%dH&%#%x%(%)]*)" )
									if type( tonumber( tag_val ) ) == "number" then -- %-?[%d&]^*[%.%dH&%#%x%(%)]*
										tag_val = tonumber( tag_val )
									end
									for k = 1, #tags_default_tags do
										if tag_nam:match( tags_default_tags[ k ] ) then
											tag_idx = k
										end
									end
									if tag_idx > 0 then
										tags_ipol_ini = tags_ipol_ini .. tag_nam .. tostring( ke4.tag.ipol( ipol_ini, tags_deafult_vals[ tag_idx ], tag_val ) )
										tags_ipol_fin = tags_ipol_fin .. tag_nam .. tostring( ke4.tag.ipol( ipol_fin, tags_deafult_vals[ tag_idx ], tag_val ) )
									end
								end
								if time_fin <= 0 then
									return tags_in_t
									--"\\t(-800,-200,\\blur4\\1cR( ))"
								elseif time_ini >= l.duration then
									return ""
								elseif time_ini < 0
									and time_fin < l.duration then
									return format( "%s%s(0,%s,%s,%s)", tags_ipol_ini, tag_time, time_fin, time_acc, tags_in_t )
									--"\\t(-800,1200,\\blur4\\xshad6)"
								elseif time_ini < 0
									and time_fin >= l.duration then
									return format( "%s%s(%s,%s)", tags_ipol_ini, tag_time, time_acc, tags_ipol_fin )
									--"\\t(-600,13000,\\blur4\\xshad6)"
								elseif time_ini > 0
									and time_fin >= l.duration then
									return format( "%s(%s,%s,%s,%s)", tag_time, time_ini, l.duration, time_acc, tags_ipol_fin )
								end --"\\t(800,13000,\\blur4\\xshad6)"
							end
						end
						return tcapture
					end
				)
				-------------------------------------
				String = String:gsub( "(\\t[cdefirswx%~%-%d]*)(%b())",
					function( Tag, Capture )
						if Capture:sub( 2, -2 ):sub( 1, 4 ) == "0,0," then
							return Capture:sub( 2, -2 ):match( "\\%S+[ %S]*" )
						end
						return Tag .. Capture:gsub( ", [ 	]*\\", ",\\" )
						:gsub( "([%d&]^*) [ 	]*%)", "%1)" )
					end --:sub( 2, -2 ):match( "\\%S+[ %S]*" ) <- captura todos los tags dentro de una \\t
				) --si hay una \\t(0,0, solo retorna los tags que hay dentro
				-------------------------------------
				String = String:gsub( "(\\t[cdefirswx%~%-%d]*%()[ ]*1[ ]*,[ ]*(\\)", "%1%2" )
				-- elimina la aceleración de las transfos \\t(1,\\tags)
				-------------------------------------
				String = String:gsub( "\\(i?clip)%([ ]*(%-?%d+[%.%d ]*),[ ]*(%-?%d+[%.%d ]*),[ ]*(%-?%d+[%.%d ]*),[ ]*(%-?%d+[%.%d ]*)",
					function( clip_tag, clip_x1, clip_y1, clip_x2, clip_y2 )
						local clip_x1, clip_y1 = tonumber( clip_x1 ), tonumber( clip_y1 )
						local clip_x2, clip_y2 = tonumber( clip_x2 ), tonumber( clip_y2 )
						clip_x1, clip_x2 = math.min( clip_x1, clip_x2 ), math.max( clip_x1, clip_x2 )
						clip_y1, clip_y2 = math.min( clip_y1, clip_y2 ), math.max( clip_y1, clip_y2 )
						return format( "\\%s(%s,%s,%s,%s", clip_tag, clip_x1, clip_y1, clip_x2, clip_y2 )
					end --organiza de menor a mayor los valores de un i?clip rectangular "\\clip(200,300,1000,100)"
				)
				-------------------------------------
				String = ke4.tag.redefine( Text_Config, String )
				String = ke4.tag.do_alpha( String )
				String = ke4.tag.modify( Text_Config, String )
				-------------------------------------
				String = String:gsub( "'(&H%x+&)'", "%1" )
				-------------------------------------
				-- elimina transformaciones vacías
				String = String:gsub( "\\t[cdefirswx]*%(%d+[%.%d ]*,%d+[%.%d ]*,[%.%d%, ]*%)", "" )
				:gsub( "\\t[cdefirswx]*%(%d+[%.%d ]*,%)", "" )
				:gsub( "\\t[cdefirswx%d%~%-]*%([ ]*%)", "" )
				:gsub( "(\\t[cdefirswx%d%~%-]*%(%d+[%.%d ]*,%d+[%.%d ]*,)1.0[ ]*,", "%1" )
				:gsub( "(\\t[cdefirswx%d%~%-]*%(%d+[%.%d ]*,%d+[%.%d ]*,)1[ ]*,", "%1" )
				:gsub( "disTorT", "distort" )
				:gsub( "jiTTer",  "jitter"  )
				:gsub( "@", "" )
				:gsub( "%)M", "%)" )
				-------------------------------------
				local function tag_round( TagString )
					-- redondea los valores numéricos de los tags
					local dec_round = 3
					TagString = TagString:gsub( "(%-?%d+%.%d+)",
						function( number )
							return ke4.math.round( tonumber( number ), dec_round )
						end
					)
					return TagString
				end
				-------------------------------------
				-- gsub( "\\[^\\{}]*" ) captura los tags, todos
				return tag_round( ke4.string.change( String, "\\org%b()", 1 ) )
			end, --tag.dark
			
			HTML_to_ass = function( Text_Config, String )
				local String = String or ""
				String = tostring( String ):gsub( "%#%x%x%x%x%x%x",
					function( HTML_color )
						return ke4.color.ass( HTML_color )
					end
				)
				String = tostring( String ):gsub( "%#(%x%x)",
					function( HTML_alpha )
						return "&H" .. HTML_alpha:reverse( ) .. "&"
					end
				)
				--▼--------------------------------------------------------
				-- retorna los tags con sus valores en default: "\\blur~"
				String = ke4.tag.default2( Text_Config, String )
				-----------------------------------------------------------
				String = ke4.tag.tonumber( String )	-- corvierte algunas convenciones en número
				---------------------------------------------------------------------------
				String = ke4.tag.do_alpha( String )	-- identifica los alphas con base decimal
				return String --se aplica al texto antes de ejecutar cualquier otra función
			end,
			
			timefx = function( Text_Config, String )
				local l = Text_Config
				local tag_timesfx = { --[["\\ts", "\\tc", "\\tf", "\\tw",--]] "\\td", "\\ti" }
				--> td = transformations default tags
				--> ti = transformations inverse tags
				local times_start, times_durat, times_tag
				local function selectx( time_fx, l )
					--[[
					if time_fx == "\\ts" then
						return syl.syl_start, syl.dur
					elseif time_fx == "\\tc" then
						return char.syl_start, char.dur
					elseif time_fx == "\\tf" then
						return furi.syl_start, furi.dur
					end
					return word.word_start, word.dur
					--]]
					return l.start_time, l.duration
				end
				-------------------------------------------------------------------------
				String = String:gsub( "%.line", ".LINE" ) --> var.line.val
				:gsub( "meta%.res_x", "xres" ):gsub( "meta%.res_y", "yres" )
				:gsub( "line%.i", "l_counter" ):gsub( "line%.n", "maxil_counter" )
				:gsub( "line%.", "linefx[  ii  ]." ) --"\\td(0,500,\\frRs( meta.res_x ))( 100 * line.i )"
				:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
				:gsub( "%.LINE", ".line" )
				-------------------------------------------------------------------------
				for i = 1, #tag_timesfx do
					times_tag = tag_timesfx[ i ]
					if String:match( times_tag )
						and times_tag ~= "\\td"
						and times_tag ~= "\\ti" then
						times_start, times_durat = selectx( times_tag, l )
						if String:match( times_tag .. "%b()%b()[%-]*" ) then
							String = String:gsub( "(" .. times_tag .. "%b())(%b())([%-]*)",
								function( time_tag, time_off, sign )
									--local line = linefx[ ii ]
									local default_after = 0
									local default_durat = 0
									local default_accel = ""
									if pcall( loadstring(
											format( "return function( meta, line, x, y ) return { %s } end", time_off:sub( 2, -2 ) ) )
										) then
										local time_function = loadstring(
											format( "return function( meta, line, x, y ) return { %s } end", time_off:sub( 2, -2 ) )
										)( )
										time_function = time_function( meta, line, x, y )
										default_after = time_function[ 1 ] or 0
										default_durat = time_function[ 2 ] or 0
										default_accel = time_function[ 3 ] or ""
									end
									local time_def_i, time_def_f = 0, 0
									if time_tag:match( times_tag .. "%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
										time_tag = time_tag:gsub( times_tag .. "%(([ ]*%-?%d+[%.%d ]*),([ ]*%-?%d+[%.%d ]*),",
											function( timei, timef )
												time_def_i = tonumber( timei ) + times_start
												time_def_f = tonumber( timef ) + times_start
												return format( "%s(%s,%s,",
													times_tag, tonumber( timei ) + times_start, tonumber( timef ) + times_start
												)
											end
										)
									elseif time_tag:match( times_tag .. "%([ ]*%-?%d+[%.%d ]*," ) then
										time_tag = time_tag:gsub( times_tag .. "%(([ ]*%-?%d+[%.%d ]*),",
											function( accel )
												time_def_i, time_def_f = times_start, times_start + times_durat
												return format( "%s(%s,%s,%s,",
													times_tag, times_start, times_start + times_durat, accel
												)
											end
										)
									else
										time_def_i, time_def_f = times_start, times_start + times_durat
										time_tag = time_tag:gsub( times_tag .. "%(",
											format( "%s(%s,%s,", times_tag, times_start, times_start + times_durat )
										)
									end
									local trans_def = format( "%s(%s,%s,%s,%s)",
										times_tag, time_def_f + default_after,
										time_def_f + default_after + default_durat,
										default_accel, sign == "-" and ke4.tag.inverse( Text_Config, time_tag ) or ke4.tag.default( Text_Config, time_tag )
									)
									trans_def = trans_def:gsub( ",,", "," )
									return time_tag .. trans_def
								end
							)
						elseif String:match( times_tag .. "%b()[km]^*[dr]*" ) then
							String = String:gsub( "(" .. times_tag .. "%b())([km]^*[dr]*)",
								function( time_tag, time_type )
									local time_i = times_start
									local time_f = times_start + times_durat
									local time_d = time_f - time_i
									local time_a = ","
									if time_tag:match( times_tag .. "%(%-?%d+[%.%d]*,%-?%d+[%.%d]*,[%.%d%,]*" ) then
										time_tag = time_tag:gsub( times_tag .. "%((%-?%d+[%.%d]*),(%-?%d+[%.%d]*),([%.%d%,]*)",
											function(  timei, timef , timea )
												time_i = tonumber( timei ) + times_start
												time_f = tonumber( timef ) + times_start
												time_d = time_f - time_i
												time_a = timea
											end
										)
									elseif time_tag:match( times_tag .. "%(%-?%d+[%.%d]*," ) then
										time_tag = time_tag:gsub( times_tag .. "%((%-?%d+[%.%d]*,)",	
											function( timea )
												time_a = timea
											end
										)
									end
									local time_tags_nor = time_tag:match( "%b()" ):match( "\\%w+[ %S]*" ):sub( 1, -2 )
									local time_tags_inv = ke4.tag.inverse( Text_Config, time_tags_nor )
									local time_tags_def = ke4.tag.default( Text_Config, time_tags_nor )
									if time_type == "m"
										or time_type == "md" then
										return format( "%sx(%s,%s,%s)%sx(%s,%s,%s)",
											times_tag, time_i + 0.00 * time_d, time_i + 0.50 * time_d, time_a .. time_tags_nor,
											times_tag, time_i + 0.50 * time_d, time_i + 1.00 * time_d, time_a .. time_tags_def
										):gsub( ",,", "," ) --"\\ts(0,syl.dur + 300,0.8,\\frxyzRs(40))m"
									elseif time_type == "mrd"
										or time_type == "mdr" then
										return format( "%sx(%s,%s,%s)%sx(%s,%s,%s)%sx(%s,%s,%s)",
											times_tag, time_i + 0.00 * time_d, time_i + 0.00 * time_d, time_a .. time_tags_nor,
											times_tag, time_i + 0.00 * time_d, time_i + 0.50 * time_d, time_a .. time_tags_inv,
											times_tag, time_i + 0.50 * time_d, time_i + 1.00 * time_d, time_a .. time_tags_def
										):gsub( ",,", "," ) --"\\ts(0,syl.dur + 300,0.8,\\frxyzRs(40))mrd"
									end
									return format( "%sx(%s,%s,%s)%sx(%s,%s,%s)%sx(%s,%s,%s)",
										times_tag, time_i + 0.00 * time_d, time_i + 0.25 * time_d, time_a .. time_tags_nor,
										times_tag, time_i + 0.25 * time_d, time_i + 0.75 * time_d, time_a .. time_tags_inv,
										times_tag, time_i + 0.75 * time_d, time_i + 1.00 * time_d, time_a .. time_tags_def
									):gsub( ",,", "," ) --"\\ts(0,syl.dur + 300,0.8,\\frxyzRs(40))k"
								end
							)
						else
							if String:match( times_tag .. "%([ ]*%-?%d+[%.%d ]*,[ ]*%-?%d+[%.%d ]*," ) then
								String = String:gsub( times_tag .. "%(([ ]*%-?%d+[%.%d ]*),([ ]*%-?%d+[%.%d ]*),",
									function( timei, timef )
										return format( "%s(%s,%s,",
											times_tag, tonumber( timei ) + times_start, tonumber( timef ) + times_start
										)
									end
								)
							elseif String:match( times_tag .. "%([ ]*%-?%d+[%.%d ]*," ) then
								String = String:gsub( times_tag .. "%(([ ]*%-?%d+[%.%d ]*),",
									function( accel )
										return format( "%s(%s,%s,%s,",
											times_tag, times_start, times_start + times_durat, accel
										)
									end
								)
							else
								String = String:gsub( times_tag .. "%(",
									format( "%s(%s,%s,", times_tag, times_start, times_start + times_durat )
								)
							end
						end
					else --td ti
						if times_tag == "\\td" then
							if String:match( "\\td%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*," )
								and String:match( "%-%)" ) == nil then
								--local line = linefx[ ii ]
								local default_after = 0
								local default_durat = 0
								local default_accel = ""
								if String:match( "\\td%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*," ) then
									default_accel = String:match( "\\td%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*,[ ]*(%d+[%.%d ]*)," )
								end
								if String:match( "\\td%b()%b()" ) then
									if pcall( loadstring(
											format( "return function( meta, line, x, y ) return { %s } end",
												String:match( "\\td%b()(%b())" ):sub( 2, -2 )
											) )
										) then
										local time_function = loadstring(
											format( "return function( meta, line, x, y ) return { %s } end",
												String:match( "\\td%b()(%b())" ):sub( 2, -2 )
											)
										)( )
										time_function = time_function( meta, line, x, y )
										default_after = time_function[ 1 ] or 0
										default_durat = time_function[ 2 ] or 0
										default_accel = time_function[ 3 ] or ""
									end
									String = String:gsub( "(\\td%b())%b()", "%1" )
								end --"\\td(0,1000,\\1cR( ))(800,300,1.5)"
								String = String:gsub( "\\td%b()",
									function( capture )
										if capture:match( "\\td%([ ]*%d+[%.%d ]*,([ ]*%d+[%.%d ]*)," ) then
											local timef = capture:match( "\\td%([ ]*%d+[%.%d ]*,([ ]*%d+[%.%d ]*)," )
											local trans_d = format( "\\td(%s,%s,%s,%s-)",
												timef + default_after, timef + default_after + default_durat,
												default_accel, ke4.tag.default( Text_Config, capture )
											)
											trans_d = trans_d:gsub( ",,", "," )
											capture = capture:sub( 1, -2 ) .. "-)"
											return capture .. trans_d
										end
										return capture
									end --"\\td(0,1000,\\1cR( ))"
								)
							end
						else
							if String:match( "\\ti%([ ]*%d+[%.%d ]*,[ ]*%d+[%.%d ]*," )
								and String:match( "%-%)" ) == nil then
								--local line = linefx[ ii ]
								local default_after = 0
								local default_durat = 0
								local default_accel = ""
								if String:match( "\\ti%b()%b()" ) then
									if pcall( loadstring(
											format( "return function( meta, line, x, y ) return { %s } end",
												String:match( "\\ti%b()(%b())" ):sub( 2, -2 )
											) )
										) then
										local time_function = loadstring(
											format( "return function( meta, line, x, y ) return { %s } end",
												String:match( "\\ti%b()(%b())" ):sub( 2, -2 )
											)
										)( )
										time_function = time_function( meta, line, x, y )
										default_after = time_function[ 1 ] or 0
										default_durat = time_function[ 2 ] or 0
										default_accel = time_function[ 3 ] or ""
									end
									String = String:gsub( "(\\ti%b())%b()", "%1" )
								end --"\\ti(0,1000,\\1cR( ))(800,300,1.5)"
								String = String:gsub( "\\ti%b()",
									function( capture )
										if capture:match( "\\ti%([ ]*%d+[%.%d ]*,([ ]*%d+[%.%d ]*)," ) then
											local timef = capture:match( "\\ti%([ ]*%d+[%.%d ]*,([ ]*%d+[%.%d ]*)," )
											local trans_i = format( "\\ti(%s,%s,%s,%s-)",
												timef + default_after, timef + default_after + default_durat,
												default_accel, ke4.tag.inverse( Text_Config, capture )
											)
											trans_i = trans_i:gsub( ",,", "," )
											capture = capture:sub( 1, -2 ) .. "-)"
											return capture .. trans_i
										end
										return capture
									end -- "\\ti(0,1000,\\xyshadRs( 5, 10 ))"
								)
							end
						end
					end
				end
				String = String:gsub( "%-%)", "%)" )
				-------------------------------------------------------------------------
				String = String:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
				:gsub( "l_counter", "line.i" ):gsub( "maxil_counter", "line.n" )
				:gsub( "linefx%[  ii  %]%.", "line." )
				:gsub( "\"(&H%x+&)\"", "%1" ):gsub( ",\\\\", "\\" )
				-------------------------------------------------------------------------
				return String
			end,
			
			modify = function( Text_Config, String )
				local l = Text_Config
				local String = String or ""
				String = String:gsub( "(\\i?clip)(%b())",
					function( Tag, capture )
						if capture:match( "m %-?%d+[%.%d]* %-?%d+[%.%d]*" )
							and capture:match( "%," ) then
							--▼ captura la shape que esté dentro del i?clip
							local shape_modify = capture:match( "m %-?%d+[%.%d]* %-?%d+[%.%-%dblm ]*" )
							if capture:match( "pos" ) then
								--"\\clip(shape.circle, 'pos')"
								shape_modify = ke4.shape.centerpos( shape_modify, l.center, l.middle )
							elseif capture:match( "%,[%d ]*%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*" ) then
								--"\\clip(shape.circle, 1, 120, -86)" relativo a la posición x, y
								local pos_clip_x, pos_clip_y = capture:match( "%,[%d ]*%,[ ]*(%-?%d+[%.%d ]*)%,[ ]*(%-?%d+[%.%d ]*)" )
								shape_modify = ke4.shape.centerpos(
									shape_modify,
									l.center + tonumber( pos_clip_x ),
									l.middle + tonumber( pos_clip_y )
								)
							elseif capture:match( "%,[ ]*%-?%d+[%.%d ]*%,[ ]*%-?%d+[%.%d ]*" ) then
								--"\\clip(shape.circle, 120, -86)" relativo al 0, 0
								local pos_clip_x, pos_clip_y = capture:match( "%,[ ]*(%-?%d+[%.%d ]*)%,[ ]*(%-?%d+[%.%d ]*)" )
								shape_modify = ke4.shape.centerpos( shape_modify, tonumber( pos_clip_x ), tonumber( pos_clip_y ) )
							else
								--"\\clip(shape.circle, true)"
								shape_modify = ke4.shape.centerpos( shape_modify, l.center, l.middle )
							end
							return format( "%s(%s)", Tag, shape_modify )
						elseif type( ke4.string.toval( "{" .. capture:sub( 2, -2 ) .. "}" ) ) == "table" then
							--para i?clips rectangulares y dos valores extras para moverlos
							local Coor = ke4.string.toval( "{" .. capture:sub( 2, -2 ) .. "}" )
							if #Coor > 4 then
								local Px1, Py1, Px2, Py2, Mx, My = Coor[ 1 ], Coor[ 2 ], Coor[ 3 ], Coor[ 4 ], Coor[ 5 ], Coor[ 6 ] or 0
								capture = format( "(%s,%s,%s,%s)", Px1 + Mx, Py1 + My, Px2 + Mx, Py2 + My )
							end
						end
						return Tag .. capture
					end
				)
				return String
			end, --"\\clip(shape.circle, 1)"
			
			do_alpha = function( String )
				local tag_alpha = {
					[ 1 ] = "\\alpha(%d+[%.%d]*)",
					[ 2 ] = "\\1a(%d+[%.%d]*)",
					[ 3 ] = "\\2a(%d+[%.%d]*)",
					[ 4 ] = "\\3a(%d+[%.%d]*)",
					[ 5 ] = "\\4a(%d+[%.%d]*)",
					[ 6 ] = "\\1va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
					[ 7 ] = "\\2va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
					[ 8 ] = "\\3va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
					[ 9 ] = "\\4va%([ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)%,[ ]*(%d+[%.%d ]*)[ ]*%)",
				}
				local tag_replace = {
					[ 1 ] = "\\alpha%s",
					[ 2 ] = "\\1a%s",
					[ 3 ] = "\\2a%s",
					[ 4 ] = "\\3a%s",
					[ 5 ] = "\\4a%s",
					[ 6 ] = "\\1va(%s,%s,%s,%s)",
					[ 7 ] = "\\2va(%s,%s,%s,%s)",
					[ 8 ] = "\\3va(%s,%s,%s,%s)",
					[ 9 ] = "\\4va(%s,%s,%s,%s)",
				}
				for i = 1, 5 do
					String = String:gsub( tag_alpha[ i ],
						function( num )
							return format( tag_replace[ i ], ke4.alpha.val2ass( num ) )
						end
					)
				end
				for i = 6, 9 do
					String = String:gsub( tag_alpha[ i ],
						function( num1, num2, num3, num4 )
							return format( tag_replace[ i ],
								ke4.alpha.val2ass( num1 ), ke4.alpha.val2ass( num2 ),
								ke4.alpha.val2ass( num3 ), ke4.alpha.val2ass( num4 )
							)
						end
					)
				end
				return String
			end,
			
			tonumber = function( String )
				-- xres, yres and ratio ----------------
				local xres, yres = aegisub.video_size( )
				if not xres then
					xres, yres = 1280, 720
				end
				ratio = xres / 1280
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local String = String or ""
				local String_out = {
					[ 1 ] = "math%.bezier2move%b()",
					[ 2 ] = "text%.kana2romaji%b()",
					[ 3 ] = "%-?%d+[%.%d]*rnd[xyz]*",
					[ 4 ] = "%-?%d+[%.%d]*fr[sxyz]*",
					[ 5 ] = "%-?%d+[%.%d]*fs[cpvxy]*",
					[ 6 ] = "%-?%d+[%.%d]*f[ae]^*[xy]*",
				}
				local String_out_tbl = { }
				for i = 1, #String_out do
					-- protege segmentos del String de remplazos
					String = String:gsub( String_out[ i ],
						function( capture )
							table.insert( String_out_tbl, capture )
							return "▲"
						end
					)
				end
				-- multiplica una constante por el ratio o por frame_dur
				String = String:gsub( "(%-?%d+[%.%d]*)([rf]^*)",
					function( capture, variable )
						local varx = ratio
						if variable == "f" then
							varx = frame_dur
						end
						return tonumber( capture ) * varx
					end
				)
				for i = 1, #String_out_tbl do
					String = String:gsub( "▲", String_out_tbl[ i ], 1 )
				end
				-------------------------------------------------------------------------
				String = String:gsub( "%.line", ".LINE" ) --> var.line.val
				:gsub( "meta%.res_x", "xres" ):gsub( "meta%.res_y", "yres" )
				:gsub( "line%.i", "l_counter" ):gsub( "line%.n", "maxil_counter" )
				:gsub( "line%.", "linefx[  ii  ]." )
				:gsub( "(&H%x+&)", "\"" .. "%1" .. "\"" )
				:gsub( "%.LINE", ".line" )
				-------------------------------------------------------------------------
				-- abreviatura de la función math.polar
				local polar_tag = {
					[ 1 ] = "(p[xy]^*)(%-?%d+[%.%d]*)d(%-?%d+[%.%d]*)",
					[ 2 ] = "(p[xy]^*)(%-?%d+[%.%d]*)d(%b())",
					[ 3 ] = "(p[xy]^*)(%b())d(%-?%d+[%.%d]*)",
					[ 4 ] = "(p[xy]^*)(%b())d(%b())",
				}
				for i = 1, #polar_tag do
					String = String:gsub( polar_tag[ i ],
						function( Pxy, Angle, Distance )
							local Return = "x"
							if Pxy:match( "py" ) then
								Return = "y"
							end
							local topolar = format( "ke4.math.polar( %s, %s, \"%s\" )", Angle, Distance, Return )
							local line = linefx[ ii ]
							if pcall( loadstring( format( "return function( meta, line, x, y ) return %s end", topolar ) ) ) then
								local fun_polar = loadstring( format( "return function( meta, line, x, y ) return %s end", topolar ) )( )
								if pcall( fun_polar ) then
									return fun_polar( meta, line, x, y )
								end
							end
							return Pxy .. Angle .. "d" .. Distance
						end
					)
				end --px45d100 = math.polar( 45, 100, "x" )
				-------------------------------------------------------------------------
				String = String:gsub( "xres", "meta.res_x" ):gsub( "yres", "meta.res_y" )
				:gsub( "l_counter", "line.i" ):gsub( "maxil_counter", "line.n" )
				:gsub( "linefx%[  ii  %]%.", "line." )
				:gsub( "\"(&H%x+&)\"", "%1" )
				-------------------------------------------------------------------------
				return String
			end,
			
			cyclic = function( j, maxj, Dur, Dur_tr, Delay, Fad_i, Fad_f, tags_ini, tags_fin )
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local tag_dur = Dur or 5000--fx.dur
				local dur_tra = Dur_tr or 2 * frame_dur
				local tra_del = Delay or 0
				local fad_ini = Fad_i or 1
				local fad_fin = Fad_f or dur_tra / 2
				local ext_ini = tags_ini or ""
				local ext_fin = tags_fin or ""
				local tag_ini = 0
				local tag_fin = tag_dur
				local tag_acc = 1
				if type( tag_dur ) == "table" then
					tag_ini = tag_dur[ 1 ]
					tag_fin = tag_dur[ 2 ]
				end
				tag_dur = tag_fin - tag_ini
				if type( dur_tra ) == "table" then
					tag_acc = dur_tra[ 2 ]
					dur_tra = dur_tra[ 1 ]
				end
				local times_tra = { }
				local durat_ini = tag_dur
				local k, i = 0, 0
				while durat_ini > 0 do
					times_tra[ k + 1 ] = {
						[ 1 ] = tag_ini + k * dur_tra,
						[ 2 ] = tag_ini + k * dur_tra + fad_ini,
						[ 3 ] = tag_ini + (k + 1) * dur_tra + tra_del - fad_fin,
						[ 4 ] = tag_ini + (k + 1) * dur_tra + tra_del
					}
					durat_ini = durat_ini - dur_tra
					k = k + 1
				end
				local tag_cyclic = "\\alpha&HFF&"
				while times_tra[ j + i * maxj ] do
					if type( ext_ini ) == "function" then
						ext_ini = ext_ini( i )
					end
					if type( ext_fin ) == "function" then
						ext_fin = ext_fin( i )
					end
					tag_cyclic = tag_cyclic .. format( "\\t(%s,%s,%s,%s\\alpha&H00&)\\t(%s,%s,%s,%s\\alpha&HFF&)",
						times_tra[ j + i * maxj ][ 1 ], times_tra[ j + i * maxj ][ 2 ], tag_acc, ext_ini,
						times_tra[ j + i * maxj ][ 3 ], times_tra[ j + i * maxj ][ 4 ], tag_acc, ext_fin
					)
					i = i + 1
				end
				return tag_cyclic
			end, --!_G.ke4.tag.cyclic( j, maxj, line.duration, 60, 240, 60, 60 )!
			
			sec = function( j, maxj, Dur, Dur_tr, tags_ini, tags_fin )
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local tag_dur = Dur or 5000--fx.dur
				local dur_tra = Dur_tr or 4 * frame_dur
				local ext_ini = tags_ini or ""
				local ext_fin = tags_fin or ""
				local tag_ini = 0
				local tag_fin = tag_dur
				if type( tag_dur ) == "table" then
					tag_ini = tag_dur[ 1 ]
					tag_fin = tag_dur[ 2 ]
				end
				tag_dur = tag_fin - tag_ini
				local dur_off = dur_tra
				local dur_int = 0
				if type( dur_tra ) == "table" then
					dur_int = dur_tra[ 3 ] or 0
					dur_off = dur_tra[ 2 ] or 0
					dur_tra = dur_tra[ 1 ]
				end
				local times_tra = { }
				local durat_ini = tag_dur
				local dur_tra_i, dur_tra_f = dur_tra, dur_tra
				if type( dur_tra ) == "table" then
					dur_tra_i = dur_tra[ 1 ]
					dur_tra_f = dur_tra[ 2 ]
				end
				local k, i = 0, 0
				while durat_ini > 0 do
					times_tra[ k + 1 ] = {
						[ 1 ] = tag_ini + k * dur_off,
						[ 2 ] = tag_ini + k * dur_off + dur_tra_i,
						[ 3 ] = tag_ini + k * dur_off + dur_tra_i + dur_int,
						[ 4 ] = tag_ini + k * dur_off + dur_tra_i + dur_int + dur_tra_f
					}
					durat_ini = durat_ini - dur_off
					k = k + 1
				end
				local tag_cyclic = ""
				local tags_tr_ini, tags_tr_fin
				while times_tra[ j + i * maxj ] do
					if type( ext_ini ) == "function" then
						ext_ini = ext_ini( i )
					end
					if type( ext_fin ) == "function" then
						ext_fin = ext_fin( i )
					end
					tags_tr_ini, tags_tr_fin = ext_ini, ext_fin
					if type( ext_ini ) == "table" then
						tags_tr_ini = ext_ini[ i % #ext_ini + 1 ]
					end
					if type( ext_fin ) == "table" then
						tags_tr_fin = ext_fin[ i % #ext_fin + 1 ]
					end
					tag_cyclic = tag_cyclic .. format( "\\t(%s,%s,%s)\\t(%s,%s,%s)",
						times_tra[ j + i * maxj ][ 1 ], times_tra[ j + i * maxj ][ 2 ], tags_tr_ini,
						times_tra[ j + i * maxj ][ 3 ], times_tra[ j + i * maxj ][ 4 ], tags_tr_fin
					)
					i = i + 1
				end
				return tag_cyclic
			end, --tag.sec( j, maxj, line.duration, { { 100, 200 }, 40 }, "\\1c&H0000FF&", "\\1c&HFFFFFF&" )
			
		},
		
		-- Color sublibrary
		color = {
			ass = function( html_color )
				local html_color = html_color or "#FFFFFF"
				local r_ass, g_ass, b_ass = html_color:match( "(%x%x)(%x%x)(%x%x)" )
				return ke4.color.val2ass( tonumber( r_ass, 16 ), tonumber( g_ass, 16 ), tonumber( b_ass, 16 ) )
			end, --!_G.ke4.color.ass( "#FF0000" )!
			
			ass2 = function( Rnum, Gnum, Bnum )
				local Rnum = Rnum or 255
				local Gnum = Gnum or 255
				local Bnum = Bnum or 255
				if type( Rnum ) == "table" then
					Bnum = Rnum[ 3 ]
					Gnum = Rnum[ 2 ]
					Rnum = Rnum[ 1 ]
				end
				Rnum = ke4.math.i( Rnum + 1, 0, 255 )[ "A-->B-->A" ]
				Gnum = ke4.math.i( Gnum + 1, 0, 255 )[ "A-->B-->A" ]
				Bnum = ke4.math.i( Bnum + 1, 0, 255 )[ "A-->B-->A" ]
				return ke4.color.val2ass( Rnum, Gnum, Bnum )
			end, -- \1c!_G.ke4.color.ass2( 255, 0, 0 )!
			
			ass3 = function( Hnum, Snum, Vnum )
				local Hnum = Hnum or 360
				local Snum = Snum or 1
				local Vnum = Vnum or 1
				if type( Hnum ) == "table" then
					Vnum = Hnum[ 3 ]
					Snum = Hnum[ 2 ]
					Hnum = Hnum[ 1 ]
				end
				Hnum = Hnum % 361
				Snum = (100 * Snum) % 101
				Vnum = (100 * Vnum) % 101
				return ke4.color.HSV_to_RGB( Hnum, Snum / 100, Vnum / 100 )
			end, -- \1c!_G.ke4.color.ass3( 15 * syl.i, 1, 1 )!
			
			rgb = function( Color_or_Table, Matrix13, Multi )
				local colorass = Color_or_Table-- or text.color1
				local matrixsm = Matrix13 or { 0, 0, 0 }
				if type( colorass ) == "table" then
					local rgb_tables, rgb_colors = { }, { }
					local rgb_r, rgb_g, rgb_b
					for i = 1, #colorass do
						rgb_tables[ i ] = ke4.color.to_RGB( colorass[ i ] )
						rgb_colors[ i ] = ke4.color.ass2( unpack( ke4.math.matrix_sum( rgb_tables[ i ], Matrix13 ) ) )
						if Multi then
							rgb_r = rgb_tables[ i ][ 1 ] * matrixsm[ 1 ]
							rgb_g = rgb_tables[ i ][ 2 ] * matrixsm[ 2 ]
							rgb_b = rgb_tables[ i ][ 3 ] * matrixsm[ 3 ]
							rgb_colors[ i ] = ke4.color.ass2( rgb_r, rgb_g, rgb_b )
						end
					end
					return rgb_colors
				end
				local rgb_table = ke4.color.to_RGB( colorass )
				if Multi then
					return ke4.color.ass2( rgb_table[ 1 ] * matrixsm[ 1 ], rgb_table[ 2 ] * matrixsm[ 2 ], rgb_table[ 3 ] * matrixsm[ 3 ] )
				end
				return ke4.color.ass2( unpack( ke4.math.matrix_sum( rgb_table, Matrix13 ) ) )
			end, --\1c!_G.ke4.color.rgb( "&H0000FF&", { 0, 50 * syl.i, 23 * syl.i } )!
			
			hsv = function( Color_or_Table, Matrix13, Multi )
				local colorass = Color_or_Table-- or text.color1
				local matrixsm = Matrix13 or { 0, 0, 0 }
				if type( colorass ) == "table" then
					local hsv_tables, hsv_colors = { }, { }
					local hsv_h, hsv_s, hsv_v
					for i = 1, #colorass do
						hsv_tables[ i ] = ke4.color.to_HSV( colorass[ i ] )
						hsv_colors[ i ] = ke4.color.ass3( unpack( ke4.math.matrix_sum( hsv_tables[ i ], Matrix13 ) ) )
						if Multi then
							hsv_h = hsv_tables[ i ][ 1 ] * matrixsm[ 1 ]
							hsv_s = hsv_tables[ i ][ 2 ] * matrixsm[ 2 ]
							hsv_v = hsv_tables[ i ][ 3 ] * matrixsm[ 3 ]
							hsv_colors[ i ] = ke4.color.ass3( hsv_h, hsv_s, hsv_v )
						end
					end
					return hsv_colors
				end
				local hsv_table = ke4.color.to_HSV( colorass )
				if Multi then
					return ke4.color.ass3( hsv_table[ 1 ] * matrixsm[ 1 ], hsv_table[ 2 ] * matrixsm[ 2 ], hsv_table[ 3 ] * matrixsm[ 3 ] )
				end
				return ke4.color.ass3( unpack( ke4.math.matrix_sum( hsv_table, Matrix13 ) ) )
			end, --\1c!_G.ke4.color.hsv( "&H0000FF&", { 17 * syl.i, 1, 0 } )!
			
			assF = function( Color_or_Table )
				local Color_or_Table = Color_or_Table-- or text.color1
				local cF, tcF = { Color_or_Table }, { }
				if type( Color_or_Table ) == "table" then
					cF = Color_or_Table
				end
				for i = 1, #cF do
					if cF[ i ]:len( ) < 15 then
						cF[ i ] = cF[ i ]:match( "%x+" )
						cF[ i ] = "&H" .. cF[ i ]:upper( ) .. "&"
					else
						for c in cF[ i ]:gmatch( "[%&Hh]*%x%x%x%x%x%x[%&]*" ) do
							c = c:match( "%x+" )
							c = format( "&H%s&", c:upper( ) )
							table.insert( tcF, c )
						end
						cF[ i ] = format( "(%s,%s,%s,%s)", tcF[ 1 ], tcF[ 2 ], tcF[ 3 ], tcF[ 4 ] )
					end
				end
				if #cF == 1 then
					return cF[ 1 ]
				end
				return cF
			end,
			
			to_RGB = function( Color_or_Table )
				local Color_or_Table = ke4.color.from_error( Color_or_Table )
				local C_ass, RGB_table, _c = { Color_or_Table }, { }, ke4.color.vc_to_c
				if type( Color_or_Table ) == "table" then
					C_ass = Color_or_Table
				end
				for i = 1, #C_ass do
					local b_RGB, g_RGB, r_RGB = _c( C_ass[ i ] ):match( "(%x%x)(%x%x)(%x%x)" )
					RGB_table[ i ] = { tonumber( r_RGB, 16 ), tonumber( g_RGB, 16 ), tonumber( b_RGB, 16 ) }
				end
				if #C_ass == 1 then
					return RGB_table[ 1 ]
				end
				return RGB_table
			end, --!_G.ke4.table.view( _G.ke4.color.to_RGB( "&HAA00FF&" ) )!
			
			to_HSV = function( Color_or_Table )
				local Color_or_Table = ke4.color.from_error( Color_or_Table )
				local c_ass, HSV_table, H, S, V, Cmin, Cmax, Dt = { Color_or_Table }, { }, 0, 0, 0, 0, 1, 1
				if type( Color_or_Table ) == "table" then
					c_ass = Color_or_Table
				end
				for i = 1, #c_ass do
					local Red, Green, Blue = unpack( ke4.color.to_RGB( c_ass[ i ] ) )
					local Rcol, Gcol, Bcol = Red / 255 + 0.000001, Green / 255, Blue / 255
					Cmax = math.max( Rcol, Gcol, Bcol )
					Cmin = math.min( Rcol, Gcol, Bcol )
					Dt = Cmax - Cmin
					H = ke4.math.round( 60 * (((Rcol - Gcol) / Dt) + 4), 3 )
					if Cmax == Rcol then
						H = ke4.math.round( 60 * (((Gcol - Bcol) / Dt) % 6), 3 )
					elseif Cmax == Gcol then
						H = ke4.math.round( 60 * (((Bcol - Rcol) / Dt) + 2), 3 )
					end
					S = ke4.math.round( Dt / Cmax, 3 )
					V = ke4.math.round( Cmax, 3 )
					HSV_table[ i ] = { H, S, V }
				end
				if #c_ass == 1 then
					return HSV_table[ 1 ]
				end
				return HSV_table
			end, --!_G.ke4.table.view( _G.ke4.color.to_HSV( "&HAA32FF&" ) )!
			
			vc = function( Color_or_Table )
				if type( Color_or_Table ) == "function" then
					Color_or_Table = Color_or_Table( )
				end
				local Color_or_Table = color.from_error( Color_or_Table or text.color1 )
				effector.print_error( Color_or_Table, "color", "color.vc", 1 )
				local vc, cvc_t = { Color_or_Table }, { }
				if type( Color_or_Table ) == "table" then
					vc = Color_or_Table
				end
				for i = 1, #vc do
					if vc[ i ]:len( ) < 15 then
						cvc_t[ i ] = math.format( "(%s,%s,%s,%s)", color.assF( vc[ i ] ) )
					else
						cvc_t[ i ] = color.assF( vc[ i ] )
					end
				end
				if #cvc_t == 1 then
					return cvc_t[ 1 ]
				end
				return cvc_t
			end,
			
			r = function( )
				return ke4.color.HSV_to_RGB( ke4.math.Rc( 360 ), ke4.math.Rc( 0, 1 ), ke4.math.Rc( 0, 1 ) )
			end, --!_G.ke4.color.r( )!

			rc = function( colorRC, ... )
				local colorRC = ke4.color.from_error( colorRC )
				local RCtable, i_c, _c = { }, ke4.color.ipolfx, ke4.color.vc_to_c
				local RCcolor = { colorRC }
				local RCmask = ke4.color.from_error( ... or { "&H6E6E6E&", "&H000000&" } )
				if type( colorRC ) == "table" then
					RCcolor = colorRC
				end
				if type( RCmask ) ~= "table" then
					RCmask = { RCmask }
				end
				if #RCmask == 1 then
					RCmask[ 2 ] = RCmask[ 1 ]
				end
				for i = 1, #RCcolor do
					RCmask = ke4.table.disorder( RCmask )
					RCtable[ i ] = i_c( ke4.math.Rc( 0.25, 1 ), i_c( ke4.math.R( 2 ) - 1, RCmask[ 1 ], RCmask[ 2 ] ), _c( RCcolor[ i ] ) )
				end
				if #RCcolor == 1 then
					return RCtable[ 1 ]
				end
				return RCtable
			end,
			
			rvc = function( colorRVC, ... )
				local colorRVC = ke4.color.from_error( colorRVC  )
				local RVCtable, RVCcolor = { }, { colorRVC }
				if type( colorRVC ) == "table" then
					RVCcolor = colorRVC
				end
				for i = 1, #RVCcolor do
					RVCtable[ i ] = format( "(%s,%s,%s,%s)",
						ke4.color.rc( RVCcolor[ i ], ... ), ke4.color.rc( RVCcolor[ i ], ... ),
						ke4.color.rc( RVCcolor[ i ], ... ), ke4.color.rc( RVCcolor[ i ], ... )
					)
				end
				if #RVCcolor == 1 then
					return RVCtable[ 1 ]
				end
				return RVCtable
			end,
			
			gradientv = function( ColorTop_or_Table, ColorBottom_or_Table )
				local ColorBottom_or_Table = ke4.color.from_error( ColorBottom_or_Table )
				local ColorTop_or_Table = ke4.color.from_error( ColorTop_or_Table )
				local Cv_table, _c = { }, ke4.color.vc_to_c
				local CT, CB = { ColorTop_or_Table }, { ColorBottom_or_Table }
				if type( ColorTop_or_Table ) == "table" then
					CT = ColorTop_or_Table
				end
				if type( ColorBottom_or_Table ) == "table" then
					CB = ColorBottom_or_Table
				end
				for i = 1, #CT do
					for k = 1, #CB do
						table.insert( Cv_table,
							format( "(%s,%s,%s,%s)",
								_c( CT[ i ] ), _c( CT[ i ] ), _c( CB[ k ] ), _c( CB[ k ] )
							)
						)
					end
				end
				if #Cv_table == 1 then
					return Cv_table[ 1 ]
				end
				return Cv_table
			end,
			
			gradienth = function( ColorLeft_or_Table, ColorRight_or_Table, val_i, val_n, algorithm )
				--example algorithm: "1 - abs( 2 * %s - 1 )"
				local Al = algorithm or "%s"
				local ColorRight_or_Table = ke4.color.from_error( ColorRight_or_Table )
				local ColorLeft_or_Table = ke4.color.from_error( ColorLeft_or_Table )
				local Ch_table, _c, i_c = { }, ke4.color.vc_to_c, ke4.color.ipolfx
				local CL, CR = { ColorLeft_or_Table }, { ColorRight_or_Table }
				local v1 = ke4.math.format( Al, 2 * (val_i - 1) / (2 * val_n - 1) )
				local v2 = ke4.math.format( Al, (2 * val_i - 1) / (2 * val_n - 1) )
				if type( ColorLeft_or_Table ) == "table" then
					CL = ColorLeft_or_Table
				end
				if type( ColorRight_or_Table ) == "table" then
					CR = ColorRight_or_Table
				end
				for i = 1, #CL do
					for k = 1, #CR do
						Cx1 = i_c( v1, _c( CL[ i ] ), _c( CR[ k ] ) )
						Cx2 = i_c( v2, _c( CL[ i ] ), _c( CR[ k ] ) )
						table.insert( Ch_table, format( "(%s,%s,%s,%s)", Cx1, Cx2, Cx1, Cx2 ) )
					end
				end
				if #Ch_table == 1 then
					return Ch_table[ 1 ]
				end
				return Ch_table
			end,
			
			vc_to_c = function( colorvc_or_table )
				local colorvc_or_table = ke4.color.from_error( colorvc_or_table )
				local colorvc, VC2Ccolors, i_c = { colorvc_or_table }, { }, ke4.color.ipolfx
				if type( colorvc_or_table ) == "table" then
					colorvc = colorvc_or_table
				end
				for k = 1, #colorvc do
					VC2Ccolors = { }
					if colorvc[ k ]:len( ) < 15 then
						colorvc[ k ] = ke4.color.assF( colorvc[ k ] )
					else
						for c in colorvc[ k ]:gmatch( "[%&Hh]*%x%x%x%x%x%x[%&]*" ) do
							table.insert( VC2Ccolors, ke4.color.assF( c ) )
						end
						colorvc[ k ] = i_c( 0.5, i_c( 0.5, VC2Ccolors[ 1 ], VC2Ccolors[ 4 ] ), i_c( 0.5, VC2Ccolors[ 2 ], VC2Ccolors[ 3 ] ) )
					end 
				end
				if #colorvc == 1 then
					return colorvc[ 1 ]
				end
				return colorvc
			end,
			
			c_to_vc = function( colorc_or_table )
				local colorc_or_table = ke4.color.from_error( colorc_or_table )
				return ke4.color.vc( colorc_or_table )
			end,
			
			interpolate = function( Color1_or_Table, Color2_or_Table, Index_Ipol )
				local II = Index_Ipol or 0.5
				local Color2_or_Table = ke4.color.from_error( Color2_or_Table )
				local Color1_or_Table = ke4.color.from_error( Color1_or_Table )
				local Ci_table, _c, i_c = { }, ke4.color.vc_to_c, ke4.color.ipolfx
				local C1, C2 = { Color1_or_Table }, { Color2_or_Table }
				if type( Color1_or_Table ) == "table" then
					C1 = Color1_or_Table
				end
				if type( Color2_or_Table ) == "table" then
					C2 = Color2_or_Table
				end
				local color1_vc, color2_vc
				for i = 1, #C1 do
					for k = 1, #C2 do
						color1_vc, color2_vc = { }, { }
						for c in C1[ i ]:gmatch( "%x%x%x%x%x%x" ) do
							table.insert( color1_vc, c )
						end
						for c in C2[ k ]:gmatch( "%x%x%x%x%x%x" ) do
							table.insert( color2_vc, c )
						end
						if #color1_vc == 1
							or #color2_vc == 1 then
							table.insert( Ci_table, i_c( II, _c( C1[ i ] ), _c( C2[ k ] ) ) )
						elseif #color1_vc == 4
							and #color2_vc == 4 then
							table.insert( Ci_table, format( "(%s,%s,%s,%s)",
									i_c( II, color1_vc[ 1 ], color2_vc[ 1 ] ),
									i_c( II, color1_vc[ 2 ], color2_vc[ 2 ] ),
									i_c( II, color1_vc[ 3 ], color2_vc[ 3 ] ),
									i_c( II, color1_vc[ 4 ], color2_vc[ 4 ] )
								)
							) --add: august 03rd 2019
						end
					end
				end
				if #Ci_table == 1 then
					return Ci_table[ 1 ]
				end
				return Ci_table
			end, --!_G.ke4.color.interpolate( "&HFFFFFF&", "&H000000&", (j - 1) / (maxj - 1) )!
			
			vector = function( Color1, Color2 )
				local Color2 = ke4.color.from_error( Color2 )
				local Color1 = ke4.color.from_error( Color1 )
				local cv_index, C1, C2 = ke4.table.disorder( 4 ), ke4.color.vc_to_c( Color1 ), ke4.color.vc_to_c( Color2 )
				local Cfx = {
					[ 1 ] = { C1, C1, C2, C1 },
					[ 2 ] = { C2, C1, C1, C1 },
					[ 3 ] = { C1, C1, C1, C2 },
					[ 4 ] = { C1, C2, C1, C1 }
				}
				local cv_table = {
					[ 0 ] = { C1, C1, C1, C1 },
					[ 1 ] = Cfx[ cv_index[ 1 ] ],
					[ 2 ] = Cfx[ cv_index[ 2 ] ],
					[ 3 ] = Cfx[ cv_index[ 3 ] ],
					[ 4 ] = { C2, C2, C2, C2 },
				}
				for i = 1, 4 do
					if cv_table[ 1 ][ i ] == cv_table[ 2 ][ i ] then
						cv_table[ 2 ][ i ] = C1
					else
						cv_table[ 2 ][ i ] = C2
					end
				end
				for i = 1, 4 do
					if cv_table[ 2 ][ i ] == cv_table[ 3 ][ i ] then
						cv_table[ 3 ][ i ] = C1
					else
						cv_table[ 3 ][ i ] = C2
					end
				end
				for i = 1, 4 do
					cv_table[ i ] = format( "(%s)", ke4.table.show( cv_table[ i ] ) )
				end
				return cv_table
			end,
			
			delay = function( time_i, delay, color_i, color_f, ... )
				local color_f = ke4.color.from_error( color_f )
				local color_i = ke4.color.from_error( color_i )
				local delay = delay or 800
				local time_i = time_i or 0
				local e_concat, ti, cd_index, _c = { ... }, time_i, ke4.table.disorder( 4 ), ke4.color.vc_to_c
				local vcl, Ci, Cf, Tag_fx, Ax, event = { }, _c( color_i ), _c( color_f ), "", "", ""
				local Cfx = {
					[ 1 ] = { Ci, Ci, Cf, Ci },
					[ 2 ] = { Cf, Ci, Ci, Ci },
					[ 3 ] = { Ci, Ci, Ci, Cf },
					[ 4 ] = { Ci, Cf, Ci, Ci },
				}
				local X = {
					[ 0 ] = { Ci, Ci, Ci, Ci },
					[ 1 ] = Cfx[ cd_index[ 1 ] ],
					[ 2 ] = Cfx[ cd_index[ 2 ] ],
					[ 3 ] = Cfx[ cd_index[ 3 ] ],
					[ 4 ] = { Cf, Cf, Cf, Cf },
				}
				for i = 1, 4 do
					if X[ 1 ][ i ] == X[ 2 ][ i ] then
						X[ 2 ][ i ] = Ci
					else
						X[ 2 ][ i ] = Cf
					end
				end
				for i = 1, 4 do
					if X[ 2 ][ i ] == X[ 3 ][ i ] then
						X[ 3 ][ i ] = Ci
					else
						X[ 3 ][ i ] = Cf
					end
				end
				if #e_concat == 0
					or e_concat == nil then
					Tag_fx = format( "\\1vc(%s)\\3vc(%s)\\4vc(%s)",
						ke4.table.show( X[ 0 ] ), ke4.table.show( X[ 0 ] ), ke4.table.show( X[ 0 ] )
					)
					for i = 1, 4 do
						Ax = ke4.table.show( X[ i ] )
						Tag_fx = Tag_fx .. format( "\\t(%s,%s,\\1vc(%s)\\3vc(%s)\\4vc(%s))",
							ti + delay * (i - 1) / 4, ti + delay * i / 4, Ax, Ax, Ax
						)
					end
				else
					for i = 1, #e_concat do
						vcl[ i ] = e_concat[ i ]
					end
					for i = 1, #e_concat do
						Tag_fx = Tag_fx .. format( "%s(%s)", vcl[ i ], ke4.table.show( X[ 0 ] ) )
					end
					for i = 1, 4 do
						Ax = ke4.table.show( X[ i ] )
						event = ""
						for k = 1, #e_concat do
							event = event .. format( "%s(%s)", vcl[ k ], Ax )
						end
						Tag_fx = Tag_fx .. format( "\\t(%s,%s,%s)",
							ti + delay * (i - 1) / 4, ti + delay  *  i / 4, event
						)
					end
				end
				return Tag_fx
			end,
			
			movedelay = function( dur, delay, mode, ... )
				local colors = ke4.color.from_error( ... )
				local mode = mode or 1
				local delay = delay or 360
				local dur = dur or fx.dur
				local CFX, CdFX, N, Tag_fx, dt, _c = { colors }, { }, { }, "", delay, ke4.color.vc_to_c
				local V, I, Ind = 1, 1, 1
				if type( colors ) == "table" then
					CFX = colors
				end
				if #CFX == 1 then
					CFX[ 2 ] = text.color1
				end
				for i = 1, #CFX - 1 do
					ke4.table.inserttable( CdFX, ke4.color.vector( _c( CFX[ i ] ), _c( CFX[ i + 1 ] ) ) )
				end
				for c_mov in tostring( mode ):gmatch( "%d" ) do
					table.insert( N, c_mov )
				end
				if #N == 0 then
					N = { [ 1 ] = 1 }
				end
				for i = 1, #N do
					N[ i ] = format( "\\%svc", N[ i ] )
				end
				CdFX = ke4.table.concat2( CdFX, N )
				local i = 0
				while dur > 0 do
					V = #CdFX - 1
					I = i + 1
					Ind = (V - V * ceil( I / V ) + I) * (-1) ^ (ceil( I / V ) + 1) + (V + 2) * (1 + (-1) ^ ceil( I / V )) / 2
					Tag_fx = Tag_fx .. format( "\\t(%s,%s,%s)",
						ke4.math.round( dt * i , 2 ), ke4.math.round( dt * (i + 1), 2 ), CdFX[ Ind ]
					)
					i = i + 1
					dur = dur - dt
				end
				return Tag_fx
			end,

			set = function( Times, Colors, Line_start_time, Line_end_time, ... )
				-- ... = \\1vc, \\3vc, \\4vc, \\1c, \\3c or \\4c
				-- frame_dur ---------------------------
				local msa, msb = aegisub.ms_from_frame( 1 ), aegisub.ms_from_frame( 101 )
				if msb then
					frame_dur = ke4.math.round( ( msb - msa ) / 100, 3 )
				end
				----------------------------------------
				local e_concat = { ... }
				local Cset_colors = ke4.color.from_error( Colors )
				local Cset_times = Times
				Cset_times = ke4.table.complete( Cset_times, Line_start_time, Line_end_time )
				local iSt = ke4.table.index( Cset_times, Line_start_time )
				local iEt = ke4.table.index( Cset_times, Line_end_time )
				local t_t = ke4.table.duplicate( Cset_times )
				if #Cset_colors >= #Cset_times - 1 then
					Cset_colors[ 0 ] = Cset_colors[ #Cset_times - 1 ]
				else
					Cset_colors[ 0 ] = "&HFFFFFF&"
				end
				local i = iSt + 1
				local Tag_fx, event, t1, t2, offset_t, c_set = "", "", 0, 1, 1, ke4.table.duplicate( Cset_colors )
				--[[
				for i = 0, #c_set do
					if type( c_set[ i ] ) == "table" then
						Cset_colors[ i ] = c_set[ i ][ (val_i - 1) % #Cset_colors[ i ] + 1 ]
					elseif type( c_set[ i ] ) == "function" then
						Cset_colors[ i ] = c_set[ i ]( )
					end
				end
				--]]
				if #e_concat == 0 then
					Tag_fx = "\\1c" .. Cset_colors[ i - 2 ]
				else
					for k = 1, #e_concat do
						if i - 2 == 0 then
							if e_concat[ k ] == "\\1c"
								or e_concat[ k ] == "\\1vc" then
								Cset_colors[ 0 ] = "&HFFFFFF&"
								if #Cset_colors >= #Cset_times - 1 then
									Cset_colors[ 0 ] = Cset_colors[ #Cset_times - 1 ]
								end
							elseif e_concat[ k ] == "\\3c"
								or e_concat[ k ] == "\\3vc" then
								Cset_colors[ 0 ] = "&H0000FF&"
								if #Cset_colors >= #Cset_times - 1 then
									Cset_colors[ 0 ] = Cset_colors[ #Cset_times - 1 ]
								end
							elseif e_concat[ k ] == "\\4c"
								or e_concat[ k ] == "\\4vc" then
								Cset_colors[ 0 ] = "&H000000&"
								if #Cset_colors >= #Cset_times - 1 then
									Cset_colors[ 0 ] = Cset_colors[ #Cset_times - 1 ]
								end
							end
						end
						Tag_fx = Tag_fx .. e_concat[ k ] .. Cset_colors[ i - 2 ]
					end
				end
				for i = 1, #Cset_times do
					if type( Cset_times[ i ] ) == "table" then
						Cset_times[ i ] = Cset_times[ i ][ 1 ]
					end
				end
				if iSt + 1 ~= iEt then
					while Cset_times[ i ] < Line_end_time do
						if type( t_t[ i ] ) == "table" then
							offset_t = t_t[ i ][ 2 ]
						end
						t1 = Cset_times[ i ] - Line_start_time - frame_dur / 2
						if t1 < 0 then
							t1 = 0
						end
						t2 = t1 + offset_t
						if #e_concat == 0 then
							Tag_fx = Tag_fx .. format( "\\t(%s,%s,\\1c%s)", t1, t2, Cset_colors[ i - 1 ] )
						else
							event = ""
							for k = 1, #e_concat do
								event = event .. e_concat[ k ] .. Cset_colors[ i - 1 ]
							end
							Tag_fx = Tag_fx .. format( "\\t(%s,%s,%s)", t1, t2, event )
						end
						i = i + 1
					end
				end
				return Tag_fx
			end,

			mask = function( Mode, Color, Mask )
				return ke4.table.mask( Color, Mask, Mode )
				--se puede usar: color.mask( mode, table, mask )[ valor ]
			end,
			
			movemask = function( Dur, Delay, Mode, Color, Mask, val_i )
				--Example: color.movemask( fx.dur, 220, "\\1c", table.make( "color", syl.n, 15, 90 )[ syl.i ], "&H000000&" )
				if type( Dur ) == "table" then
					table.insert( Dur, val_i + 1 )
				else
					Dur = { 0, Dur, val_i + 1 }
				end
				local CmMtb = ke4.table.mask( Color, Mask, Mode, true )
				return CmMtb[ val_i ] .. ke4.tag.oscill( Dur, Delay, CmMtb )
			end, --color.movemask( fx.dur, 300, "\\1c", "&H0000FF&" )

			masktable = function( Color_or_Table, Size, val_i )
				local Color_or_Table = ke4.color.from_error( Color_or_Table )
				local Ct, tone, TT_cmask
				local T_cmask = recall.cmaskt
				local Size = Size or val_n
				if val_i == 1 then
					Ct = Color_or_Table
					T_cmask = remember( "cmaskt", { } )
					tone, TT_cmask = { }, { }
					if type( Ct ) ~= "table" then
						for i = 1, 2 * (Size + 1) do
							tone[ i ] = ke4.color.interpolate( Ct, "&H000000&", ke4.math.Rc( 0, 0.8 ) )
						end
						for k = 1, Size do
							T_cmask[ k ] = format( "(%s,%s,%s,%s)",
								tone[ 2 * k - 1 ], tone[ 2 * k + 1 ],
								tone[ 2 * k - 0 ], tone[ 2 * k + 2 ]
							)
						end
					else
						for i = 1, #Ct do
							tone[ i ], T_cmask[i] = { }, { }
							for k = 1, 2 * (Size + 1) do
								tone[ i ][ k ] = ke4.color.interpolate( Ct[ i ], "&H000000&", ke4.math.Rc( 0, 0.8 ) )
							end
							for h = 1, Size do
								T_cmask[ i ][ h ] = format( "(%s,%s,%s,%s)",
									tone[ i ][ 2 * h - 1 ], tone[ i ][ 2 * h + 1 ],
									tone[ i ][ 2 * h - 0 ], tone[ i ][ 2 * h + 2 ]
								)
							end
						end
						for i = 1, #Ct do
							TT_cmask[ i ] = T_cmask[ i ]
						end
						T_cmask = TT_cmask
					end
				end
				return T_cmask
			end,
			
			ipol = function( val_i, val_n, ... )
				local colors_tbl = { ... }
				if type( ... ) == "table" then
					colors_tbl = ... 
				end
				if #colors_tbl == 1 then
					colors_tbl[ 2 ] = "&HFFFFFF&"
				end
				local color_ipol = recall.c_ipol
				local max_ipol = val_n - 1
				local ipol_i, ipol_f, pct_ip
				if val_n == 1 then
					return colors_tbl[ 1 ]
				end
				if val_i == 1 then
					color_ipol = remember( "c_ipol", { } )
					for i = 1, max_ipol do
						ipol_i = colors_tbl[ floor( (i - 1) / (max_ipol / (#colors_tbl - 1)) ) + 1 ]
						ipol_f = colors_tbl[ floor( (i - 1) / (max_ipol / (#colors_tbl - 1)) ) + 2 ]
						pct_ip = floor( (i - 1) % (max_ipol / (#colors_tbl - 1)) ) / (max_ipol / (#colors_tbl - 1))
						color_ipol[ i ] = ke4.color.interpolate( ipol_i, ipol_f, pct_ip )
					end
					color_ipol[ #color_ipol + 1 ] = colors_tbl[ #colors_tbl ]
				end
				return color_ipol[ val_i ]
			end, --color.ipol( "&H00FF00&", "&HFF0000&", "&H0000FF&" )

			loop = function( j, maxj, ... )
				local colors_tbl = { ... }
				if type( ... ) == "table" then
					colors_tbl = ... 
				end
				if #colors_tbl == 1 then
					colors_tbl[ 2 ] = "&HFFFFFF&"
				end
				local color_loop = recall.c_loop
				local max_loop = maxj - 1
				local ipol_i, ipol_f, pct_ip
				if maxj == 1 then
					return colors_tbl[ 1 ]
				end
				if fx.filter == "mod" then
					max_loop = maxj
				end
				if j == 1 then
					color_loop = remember( "c_loop", { } )
					for i = 1, max_loop do
						ipol_i = colors_tbl[ floor( (i - 1) / (max_loop / (#colors_tbl - 1)) ) + 1 ]
						ipol_f = colors_tbl[ floor( (i - 1) / (max_loop / (#colors_tbl - 1)) ) + 2 ]
						pct_ip = floor( (i - 1) % (max_loop / (#colors_tbl - 1)) ) / (max_loop / (#colors_tbl - 1))
						color_loop[ i ] = ke4.color.interpolate( ipol_i, ipol_f, pct_ip )
					end
					color_loop[ #color_loop + 1 ] = colors_tbl[ #colors_tbl ]
				end
				return color_loop[ j ]
			end, --color.loop( "&H00FF00&", "&HFF0000&", "&H0000FF&" )

			bigradient = function( Color1_or_Table, Color2_or_Table, Size_Table, val_i )
				local  CbGgt = ke4.table.bigradient( Color1_or_Table, Color2_or_Table, Size_Table )
				return CbGgt[ (val_i - 1) % #CbGgt + 1 ]
			end,
			
			from_error = function( Color_or_Table )
				if type( Color_or_Table ) == "string" then
					if Color_or_Table:match( "#%x%x%x%x%x%x" ) then
						Color_or_Table = Color_or_Table:gsub( "#%x%x%x%x%x%x",
							function( HTML_color )
								return ke4.color.ass( HTML_color )
							end
						)
					else
						Color_or_Table = Color_or_Table:gsub( "[%&Hh]*(%x%x%x%x%x%x)[%&]*",
							function( ASS_color )
								return format( "&H%s&", ASS_color )
							end
						)
					end
				elseif type( Color_or_Table ) == "table" then
					for k, valor in pairs( Color_or_Table ) do
						if type( valor ) == "string" then
							if valor:match( "#%x%x%x%x%x%x" ) then
								Color_or_Table[ k ] = valor:gsub( "#%x%x%x%x%x%x",
									function( HTML_color )
										return ke4.color.ass( HTML_color )
									end
								)
							else
								Color_or_Table[ k ] = valor:gsub( "[%&Hh]*(%x%x%x%x%x%x)[%&]*",
									function( ASS_color )
										return format( "&H%s&", ASS_color )
									end
								)
							end
						end
					end
				end
				return Color_or_Table
			end,
			
			to_HTML = function( ASScolor )
				if type( ASScolor ) == "function" then
					ASScolor = ASScolor( )
				end
				local ASScolor = color.vc_to_c( ASScolor or "&HFFFFFF&" )
				local Bhtml, Ghtml, Rhtml = ASScolor:match( "(%x%x)(%x%x)(%x%x)" )
				return format( "#%s%s%s", Rhtml, Ghtml, Bhtml )
			end,

			matrix = function( Color_or_Table, ... )
				local Colorx = Color_or_Table
				local Matrixes = { ... }
				if #Matrixes == 0 then
					Matrixes[ 1 ] = {
						1,	0,	0,
						0,	1,	0,
						0,	0,	1
					}
				end
				for i = 1, #Matrixes do
					if #Matrixes[ i ] ~= 9 then
						error( "<<Error: color.matrix>> Cada matriz debe ser de tamaño 3x3\n3x3 matrix expected", 2 )
					end
					for _, v in ipairs( Matrixes[ i ] ) do
						if type( v ) ~= "number" then
							error( "<<Error: color.matrix>> Cada matriz solo debe contener números\nmatrix must contain only numbers", 2 )
						end
					end
				end
				if type( Colorx ) == "table" then
					local RGB_tables, Col_matrix = { }, { }
					for i = 1, #Colorx do
						RGB_tables[ i ] = ke4.color.to_RGB( Colorx[ i ] )
						Col_matrix[ i ] = ke4.color.ass2( ke4.math.matrix_mul( RGB_tables[ i ], ke4.math.matrix_mul2( unpack( Matrixes ) ) ) )
					end
					return Col_matrix
				end
				local RGB_table = ke4.color.to_RGB( Colorx )
				local Mtx_multi = ke4.math.matrix_mul2( unpack( Matrixes ) )
				return ke4.color.ass2( ke4.math.matrix_mul( RGB_table, Mtx_multi ) )
			end,
			
			colorchange = function( Color_or_Table, dur )
				local colors = { Color_or_Table }
				if type( Color_or_Table ) == "table" then
					colors = Color_or_Table
				end
				colors = ke4.color.from_error( colors )
				local t1, tc = 0, "\\1c"
				local t2 = dur or 5000
				if type( dur ) == "table" then
					t1 = dur[ 1 ]
					t2 = dur[ 2 ]
				end
				local i, tagfx, durt = 1, "", ke4.math.round( (t2 - t1) / #colors, 2 )
				while i <= #colors do
					tagfx = tagfx .. format( "\\t(%s,%s,%s)",
						t1 + durt * (i - 1), t1 + durt * i, tc .. ke4.color.assF( colors[ i ] )
					)
					i = i + 1
				end
				return tagfx
			end,
			
			fromstyle = function( ColorAlpha )
				--color from style
				local toColor = ""
				if ColorAlpha:match( "%x%x%x%x%x%x%x%x" ) then
					toColor = "&H" .. ColorAlpha:match( "%x%x(%x%x%x%x%x%x)" ) .. "&"
				elseif ColorAlpha:match( "%x%x%x%x%x%x" ) then
					toColor = ColorAlpha
				end
				return toColor
			end,
			
			val2ass = function( val_R, val_G, val_B )
				local col_R = ke4.math.to16( ke4.math.round( val_R ) )
				local col_G = ke4.math.to16( ke4.math.round( val_G ) )
				local col_B = ke4.math.to16( ke4.math.round( val_B ) )
				if col_R:len( ) == 1 then
					col_R = "0" .. col_R
				end
				if col_G:len( ) == 1 then
					col_G = "0" .. col_G
				end
				if col_B:len( ) == 1 then
					col_B = "0" .. col_B
				end
				return "&H" .. col_B .. col_G .. col_R .. "&"
			end, --!_G.ke4.color.val2ass( 255, 0, 0 )!

			ipolfx = function( Ipol, Color1, Color2 )
				---------------------------------------------------------------
				if Color1:match( "%x%x%x%x%x%x%x%x" ) then
					Color1 = Color1:match( "%x%x(%x%x%x%x%x%x)" )
				end
				local col_R1 = tonumber( Color1:match( "%x%x%x%x(%x%x)" ), 16 )
				local col_G1 = tonumber( Color1:match( "%x%x(%x%x)%x%x" ), 16 )
				local col_B1 = tonumber( Color1:match( "(%x%x)%x%x%x%x" ), 16 )
				---------------------------------------------------------------
				if Color2:match( "%x%x%x%x%x%x%x%x" ) then
					Color2 = Color2:match( "%x%x(%x%x%x%x%x%x)" )
				end
				local col_R2 = tonumber( Color2:match( "%x%x%x%x(%x%x)" ), 16 )
				local col_G2 = tonumber( Color2:match( "%x%x(%x%x)%x%x" ), 16 )
				local col_B2 = tonumber( Color2:match( "(%x%x)%x%x%x%x" ), 16 )
				---------------------------------------------------------------
				local Ipol = Ipol or 0.5
				Ipol = ke4.math.clamp( Ipol )
				local ipol_R = ke4.math.round( col_R1 + (col_R2 - col_R1) * Ipol )
				local ipol_G = ke4.math.round( col_G1 + (col_G2 - col_G1) * Ipol )
				local ipol_B = ke4.math.round( col_B1 + (col_B2 - col_B1) * Ipol )
				return ke4.color.val2ass( ipol_R, ipol_G, ipol_B )
			end, --!_G.ke4.color.ipolfx( 0.5, "&HFFFFFF&", "&H0000FF&" )!
			
			HSV_to_RGB = function( Hue, Saturation, Value )
				--HSV to ass color format :D HSV2ass
				local H = ((Hue - 1) % 360 + 1) / 360 * 6
				local S = ke4.math.clamp( Saturation )
				local V = ke4.math.clamp( Value )
				if S == 0 then
					return "&HFFFFFF&"
				end
				if V == 0 then
					return "&H000000&"
				end
				local C = V * S
				local M = V - C
				local X = C * (1 - abs( (H % 2) - 1 ))
				local Rx, Gx ,Bx = 0, 0, 0
				if H < 1 then
					Rx, Gx ,Bx  = C, X, 0
				elseif H < 2 then
					Rx, Gx ,Bx  = X, C, 0
				elseif H < 3 then
					Rx, Gx ,Bx  = 0, C, X
				elseif H < 4 then
					Rx, Gx ,Bx  = 0, X, C
				elseif H < 5 then
					Rx, Gx ,Bx  = X, 0, C
				else
					Rx, Gx ,Bx  = C, 0, X
				end
				return ke4.color.val2ass( 255 * (Rx + M), 255 * (Gx + M), 255 * (Bx + M) )
			end, --!_G.ke4.color.HSV_to_RGB( 0, 1, 1 )!

		},
		
		-- Alpha sublibrary
		alpha = {
			from_error = function( Alpha_or_Table )
				if type( Alpha_or_Table ) == "string" then
					Alpha_or_Table = Alpha_or_Table:gsub( "[%#%&Hh]*(%x%x)[%&]*",
						function( ASS_alpha )
							return format( "&H%s&", ASS_alpha )
						end
					)
				elseif type( Alpha_or_Table ) == "table" then
					for k, valor in pairs( Alpha_or_Table ) do
						if type( valor ) == "string" then
							Alpha_or_Table[ k ] = valor:gsub( "[%#%&Hh]*(%x%x)[%&]*",
								function( ASS_alpha )
									return format( "&H%s&", ASS_alpha )
								end
							)
						end
					end
				end
				return Alpha_or_Table
			end,
			
			assF = function( Alpha_or_Table )
				local Alpha_or_Table = ke4.alpha.from_error( Alpha_or_Table or "&H00&" )
				local aF, taF = { Alpha_or_Table }, { }
				if type( Alpha_or_Table ) == "table" then
					aF = Alpha_or_Table
				end
				for i = 1, #aF do
					taF = { }
					if type( aF[ i ] ) == "number" then
						aF[ i ] = ke4.alpha.val2ass( aF[ i ] % 256 )
					else
						if aF[ i ]:len( ) < 7 then
							aF[ i ] = aF[ i ]:match( "%x+" )
							aF[ i ] = format( "&H%s&", aF[ i ]:upper( ) )
						else
							for cF in aF[ i ]:gmatch( "[%&Hh]*%x%x[%&]*" ) do
								cF = cF:match( "%x+" )
								cF = format( "&H%s&", cF:upper( ) )
								table.insert( taF, cF )
							end
							aF[ i ] = format( "(%s,%s,%s,%s)", taF[ 1 ], taF[ 2 ], taF[ 3 ], taF[ 4 ] )
						end
					end
				end
				if #aF == 1 then
					return aF[ 1 ]
				end
				return aF
			end,
			
			va = function( Alpha_or_Table )
				local Alpha_or_Table = ke4.alpha.from_error( Alpha_or_Table or "&H00&" )
				local vA, ava_t = { Alpha_or_Table }, { }
				if type( Alpha_or_Table ) == "table" then
					vA = Alpha_or_Table
				end
				for i = 1, #vA do
					if type( vA[ i ] ) == "number"
						or vA[ i ]:len( ) < 7 then
						ava_t[ i ] = ke4.math.format( "(%s,%s,%s,%s)", ke4.alpha.assF( vA[ i ] ) )
					else
						ava_t[ i ] = ke4.alpha.assF( vA[ i ] )
					end
				end
				if #ava_t == 1 then
					return ava_t[ 1 ]
				end
				return ava_t
			end,

			r = function( )
				return ke4.alpha.val2ass( ke4.math.R( 0, 255 ) )
			end,
			
			ra = function( ArA_alpha, ... )
				local ArA_alpha = ke4.alpha.from_error( ArA_alpha or "&H00&" )
				local ArAmsk = ke4.alpha.from_error( ... or { "&HFF&", "&H00&" } )
				local ArAalp, ArAtbl, i_a, _a = { ArA_alpha }, { }, ke4.alpha.ipolfx, ke4.alpha.va_to_a
				if type( ArA_alpha ) == "table" then
					ArAalp = ArA_alpha
				end
				if type( ArAmsk ) ~= "table" then
					ArAmsk = { ArAmsk }
				end
				if #ArAmsk == 1 then
					ArAmsk[ 2 ] = ArAmsk[ 1 ]
				end
				for i = 1, #ArAalp do
					ArAmsk = ke4.table.disorder( ArAmsk )
					ArAtbl[ i ] = i_a( ke4.math.Rc( 0.2, 1 ), i_a( ke4.math.R( 2 ) - 1, ArAmsk[ 1 ], ArAmsk[ 2 ] ), _a( ArAalp[ i ] ) )
				end
				if #ArAalp == 1 then
					return ArAtbl[ 1 ]
				end
				return ArAtbl
			end,
			
			rva = function( ArCA_alpha, ... )
				local ArCA_alpha = ke4.alpha.from_error( ArCA_alpha or "&H00&" )
				local ArCAalp, ArCAtbl = { ArCA_alpha }, { }
				if type( ArCA_alpha ) == "table" then
					ArCAalp = ArCA_alpha
				end
				for i = 1, #ArCAalp do
					ArCAtbl[ i ] = format( "(%s,%s,%s,%s)",
						ke4.alpha.ra( ArCAalp[ i ], ... ), ke4.alpha.ra( ArCAalp[ i ], ... ),
						ke4.alpha.ra( ArCAalp[ i ], ... ), ke4.alpha.ra( ArCAalp[ i ], ... )
					)
				end
				if #ArCAalp == 1 then
					return ArCAtbl[ 1 ]
				end
				return ArCAtbl
			end,
			
			gradientv = function( AlphaTop_or_table, AlphaBottom_or_table )
				local AlphaTop_or_table = ke4.alpha.from_error( AlphaTop_or_table or "&H00&" )
				local AlphaBottom_or_table = ke4.alpha.from_error( AlphaBottom_or_table or "&HFF&" )
				local Av_table, _a = { }, ke4.alpha.va_to_a
				local AT, AB = { AlphaTop_or_table }, { AlphaBottom_or_table }
				if type( AlphaTop_or_table ) == "table" then
					AT = AlphaTop_or_table
				end
				if type( AlphaBottom_or_table ) == "table" then
					AB = AlphaBottom_or_table
				end
				for i = 1, #AT do
					for k = 1, #AB do
						table.insert( Av_table, format( "(%s,%s,%s,%s)",
							_a( AT[ i ] ), _a( AT[ i ] ), _a( AB[ k ] ), _a( AB[ k ] ) )
						)
					end
				end
				if #Av_table == 1 then
					return Av_table[ 1 ]
				end
				return Av_table
			end,
			
			gradienth = function( AlphaLeft_or_table, AlphaRight_or_table, val_i, val_n, algorithm )
				--example algorithm: "1 - abs(2 * %s - 1)"
				local AlphaLeft_or_table = ke4.alpha.from_error( AlphaLeft_or_table or "&H00&" )
				local AlphaRight_or_table = ke4.alpha.from_error( AlphaRight_or_table or "&HFF&" )
				local algorithm = algorithm or "%s"
				local Ah_table, _a, i_a = { }, ke4.alpha.va_to_a, ke4.alpha.ipolfx
				local AL, AR = { AlphaLeft_or_table }, { AlphaRight_or_table }
				local v1 = ke4.math.format( algorithm, 2 * (val_i - 1) / (2 * val_n - 1) )
				local v2 = ke4.math.format( algorithm, (2 * val_i - 1) / (2 * val_n - 1) )
				if type( AlphaLeft_or_table ) == "table" then
					AL = AlphaLeft_or_table
				end
				if type( AlphaRight_or_table ) == "table" then
					AR = AlphaRight_or_table
				end
				for i = 1, #AL do
					for k = 1, #AR do
						Ax1 = i_a( v1, _a( AL[ i ] ), _a( AR[ k ] ) )
						Ax2 = i_a( v2, _a( AL[ i ] ), _a( AR[ k ] ) )
						table.insert( Ah_table, format( "(%s,%s,%s,%s)", Ax1, Ax2, Ax1, Ax2 ) )
					end
				end
				if #Ah_table == 1 then
					return Ah_table[ 1 ]
				end
				return Ah_table
			end,
			
			va_to_a = function( Alphava_or_Table )
				local Alphava_or_Table = ke4.alpha.from_error( Alphava_or_Table or "&H00&" )
				local alphava, alphas, i_a = Alphava_or_Table, { }, ke4.alpha.ipolfx
				if type( Alphava_or_Table ) ~= "table" then
					alphava = { Alphava_or_Table }
				end
				for k = 1, #alphava do
					if type( alphava[ k ] ) == "string"
						and alphava[ k ]:len( ) >= 13 then
						alphas = { }
						for c_va in alphava[ k ]:gmatch( "[%&Hh]*%x%x[%&]*" ) do
							table.insert( alphas, ke4.alpha.assF( c_va ) )
						end
						alphava[ k ] = i_a( 0.5, i_a( 0.5, alphas[ 1 ], alphas[ 4 ] ), i_a( 0.5, alphas[ 2 ], alphas[ 3 ] ) )
					elseif type( alphava[ k ] ) == "number" then
						alphava[ k ] = ke4.alpha.assF( alphava[ k ] )
					end 
				end
				if #alphava == 1 then
					return alphava[ 1 ]
				end
				return alphava
			end,
			
			a_to_va = function( Alphaa_or_Table )
				local Alphaa_or_Table = ke4.alpha.from_error( Alphaa_or_Table or "&H00&" )
				return ke4.alpha.va( Alphaa_or_Table )
			end,
			
			interpolate = function( Alpha1_or_Table, Alpha2_or_Table, Index_Ipol )
				local II = Index_Ipol or 0.5
				local Alpha1_or_Table = ke4.alpha.from_error( Alpha1_or_Table or "&HFF&" )
				local Alpha2_or_Table = ke4.alpha.from_error( Alpha2_or_Table or "&H00&" )
				local Ai_table, _a, i_a = { }, ke4.alpha.va_to_a, ke4.alpha.ipolfx
				local A1, A2 = { Alpha1_or_Table }, { Alpha2_or_Table }
				if type( Alpha1_or_Table ) == "table" then
					A1 = Alpha1_or_Table
				end
				if type( Alpha2_or_Table ) == "table" then
					A2 = Alpha2_or_Table
				end
				local alpha1_va, alpha2_va
				for i = 1, #A1 do
					for k = 1, #A2 do
						alpha1_va, alpha2_va = { }, { }
						if type( A1[ i ] ) == "number" then
							A1[ i ] = ke4.alpha.val2ass( A1[ i ] )
						end
						if type( A2[ k ] ) == "number" then
							A2[ k ] = ke4.alpha.val2ass( A2[ k ] )
						end
						for c in A1[ i ]:gmatch( "%x%x" ) do
							table.insert( alpha1_va, c )
						end
						for c in A2[ k ]:gmatch( "%x%x" ) do
							table.insert( alpha2_va, c )
						end
						if #alpha1_va == 1
							or #alpha2_va == 1 then
							table.insert( Ai_table, i_a( II, _a( A1[ i ] ), _a( A2[ k ] ) ) )
						elseif #alpha1_va == 4
							and #alpha2_va == 4 then
							table.insert( Ai_table, format( "(%s,%s,%s,%s)",
									i_a( II, alpha1_va[ 1 ], alpha2_va[ 1 ] ),
									i_a( II, alpha1_va[ 2 ], alpha2_va[ 2 ] ),
									i_a( II, alpha1_va[ 3 ], alpha2_va[ 3 ] ),
									i_a( II, alpha1_va[ 4 ], alpha2_va[ 4 ] )
								)
							)--add: august 03rd 2019
						end
					end
				end
				if #Ai_table == 1 then
					return Ai_table[ 1 ]
				end
				return Ai_table
			end, --!_G.ke4.alpha.interpolate( "&HFF&", "&H00&", 0.75 )!

			delay = function( time_i, delay, alpha_i, alpha_f, ... )
				local time_i = time_i or 0
				local delay = delay or 640
				local alpha_i = ke4.alpha.from_error( alpha_i or "&HFF&" )
				local alpha_f = ke4.alpha.from_error( alpha_f or "&H00&" )
				local e_concat, Tag_fx, event, ti, AD_val, index = { ... }, "", "", time_i, { }, ke4.table.disorder( 4 )
				local Ai, Af = ke4.alpha.assF( alpha_i ), ke4.alpha.assF( alpha_f )
				local Afx = {
					[ 1 ] = { Ai, Ai, Af, Ai },
					[ 2 ] = { Af, Ai, Ai, Ai },
					[ 3 ] = { Ai, Ai, Ai, Af },
					[ 4 ] = { Ai, Af, Ai, Ai },
				}
				local X = {
					[ 0 ] = { Ai, Ai, Ai, Ai },
					[ 1 ] = Afx[ index[ 1 ] ],
					[ 2 ] = Afx[ index[ 2 ] ],
					[ 3 ] = Afx[ index[ 3 ] ],
					[ 4 ] = { Af, Af, Af, Af },
				}
				for i = 1, 4 do
					if X[ 1 ][ i ] == X[ 2 ][ i ] then
						X[ 2 ][ i ] = Ai
					else
						X[ 2 ][ i ] = Af
					end
				end
				for i = 1, 4 do
					if X[ 2 ][ i ] == X[ 3 ][ i ] then
						X[ 3 ][ i ] = Ai
					else
						X[ 3 ][ i ] = Af
					end
				end
				local Ax
				if #e_concat == 0
					or e_concat == nil then
					Tag_fx = format( "\\1va(%s)\\3va(%s)\\4va(%s)",
						ke4.table.show( X[ 0 ] ), ke4.table.show( X[ 0 ] ), ke4.table.show( X[ 0 ] )
					)
					for i = 1, 4 do
						Ax = ke4.table.show( X[ i ] )
						Tag_fx = Tag_fx .. format( "\\t(%s,%s,\\1va(%s)\\3va(%s)\\4va(%s))",
							ti + delay * (i - 1) / 4, ti + delay * i / 4, Ax, Ax, Ax
						)
					end
				else
					for i = 1, #e_concat do
						AD_val[ i ] = e_concat[ i ]
					end
					for i = 1, #e_concat do
						Tag_fx = Tag_fx .. format( "%s(%s)", AD_val[ i ], ke4.table.show( X[ 0 ] ) )
					end
					for i = 1, 4 do
						Ax = ke4.table.show( X[ i ] )
						event = ""
						for k = 1, #e_concat do
							event = event .. format( "%s(%s)", AD_val[ k ], Ax )
						end
						Tag_fx = Tag_fx .. format( "\\t(%s,%s,%s)",
							ti + delay * (i - 1) / 4, ti + delay * i / 4, event
						)
					end
				end
				return Tag_fx
			end,
			
			mask = function( Mode, Alpha, Mask )
				return ke4.table.mask( Alpha, Mask, Mode )
			end,
			
			ipol = function( Size, ... )
				local alphas_tbl = { ... }
				if type( ... ) == "table" then
					alphas_tbl = ... 
				end
				if #alphas_tbl == 1 then
					alphas_tbl[ 2 ] = text.alpha1
				end
				local alpha_ipol = { }
				local max_ipol = Size - 1
				local ipol_i, ipol_f, pct_ip
				if Size == 1 then
					return alphas_tbl[ 1 ]
				end
				for i = 1, max_ipol do
					ipol_i = alphas_tbl[ floor( (i - 1) / (max_ipol / (#alphas_tbl - 1)) ) + 1 ]
					ipol_f = alphas_tbl[ floor( (i - 1) / (max_ipol / (#alphas_tbl - 1)) ) + 2 ]
					pct_ip = floor( (i - 1) % (max_ipol / (#alphas_tbl - 1)) ) / (max_ipol / (#alphas_tbl - 1))
					alpha_ipol[ i ] = ke4.alpha.interpolate( ipol_i, ipol_f, pct_ip )
				end
				alpha_ipol[ #alpha_ipol + 1 ] = alphas_tbl[ #alphas_tbl ]
				return alpha_ipol
			end,

			loop = function( j, maxj, ... )
				local alphas_tbl = { ... }
				if type( ... ) == "table" then
					alphas_tbl = ... 
				end
				if #alphas_tbl == 1 then
					alphas_tbl[ 2 ] = text.alpha1
				end
				local alpha_loop = { }
				local max_loop = maxj - 1
				local ipol_i, ipol_f, pct_ip
				if maxj == 1 then
					return alphas_tbl[ 1 ]
				end
				for i = 1, max_loop do
					ipol_i = alphas_tbl[ floor( (i - 1) / (max_loop / (#alphas_tbl - 1)) ) + 1 ]
					ipol_f = alphas_tbl[ floor( (i - 1) / (max_loop / (#alphas_tbl - 1)) ) + 2 ]
					pct_ip = floor( (i - 1) % (max_loop / (#alphas_tbl - 1)) ) / (max_loop / (#alphas_tbl - 1))
					alpha_loop[ i ] = ke4.alpha.interpolate( ipol_i, ipol_f, pct_ip )
				end
				alpha_loop[ #alpha_loop + 1 ] = alphas_tbl[ #alphas_tbl ]
				return alpha_loop[ j ]
			end,

			bigradient = function( Alpha1_or_Table, Alpha2_or_Table, Size_Table, val_i )
				local Alpha1 = ke4.alpha.from_error( Alpha1_or_Table or "&H00&" )
				local Alpha2 = ke4.alpha.from_error( Alpha2_or_Table or "&HFF&" )
				local bigrad = ke4.table.bigradient( Alpha1, Alpha2, Size_Table )
				return bigrad[ (val_i - 1) % #bigrad + 1 ]
			end,
			
			fromstyle = function( ColorAlpha )
				--alpha from style
				local toAlpha = ""
				if ColorAlpha:match( "%x%x%x%x%x%x%x%x" ) then
					toAlpha = "&H" .. ColorAlpha:match( "(%x%x)%x%x%x%x%x%x" ) .. "&"
				elseif ColorAlpha:match( "%x%x" ) then
					toAlpha = ColorAlpha
				end
				return toAlpha
			end,
			
			val2ass = function( val_A )
				--number to alpha
				local val_A = ke4.math.clamp( val_A, 0, 255 )
				local alpha_A = ke4.math.to16( ke4.math.round( val_A ) )
				if alpha_A:len( ) == 1 then
					alpha_A = "0" .. alpha_A
				end
				return "&H" .. alpha_A .. "&"
			end, --!_G.ke4.alpha.val2ass( 86 )!
			
			ipolfx = function( Ipol, Alpha1, Alpha2 )
				local alpha_i, alpha_f = 0, 255
				if tonumber( Alpha1 ) then
					Alpha1 = tonumber( Alpha1 )
				end
				if tonumber( Alpha2 ) then
					Alpha2 = tonumber( Alpha2 )
				end
				-------------------------------------------------------
				if type( Alpha1 ) == "string"
					and Alpha1:match( "%x%x%x%x%x%x%x%x" ) then
					Alpha1 = Alpha1:match( "(%x%x)%x%x%x%x%x%x" )
				end
				if type( Alpha1 ) == "string" then
					alpha_i = tonumber( Alpha1:match( "(%x%x)" ), 16 )
				end
				if type( Alpha1 ) == "number" then
					alpha_i = ke4.math.clamp( Alpha1, 0, 255 )
				end
				-------------------------------------------------------
				if type( Alpha2 ) == "string"
					and Alpha2:match( "%x%x%x%x%x%x%x%x" ) then
					Alpha2 = Alpha2:match( "(%x%x)%x%x%x%x%x%x" )
				end
				if type( Alpha2 ) == "string" then
					alpha_f = tonumber( Alpha2:match( "(%x%x)" ), 16 )
				end
				if type( Alpha2 ) == "number" then
					alpha_f = ke4.math.clamp( Alpha2, 0, 255 )
				end
				-------------------------------------------------------
				local Ipol = Ipol or 0.5
				Ipol = ke4.math.clamp( Ipol )
				return ke4.alpha.val2ass( ke4.math.round( alpha_i + (alpha_f - alpha_i) * Ipol ) )
			end, --!_G.ke4.alpha.ipolfx( 0.5, "&HFF&", 55 )!

		},
		
		-- File sublibrary
		file = {
			to_auto4 = function( File_lua )
				local File_lua = File_lua or "my.ass"
				local Lines = ke4.file.get_lines( File_lua, 1 )
				local auto4_tbl = Lines[ 1 ]:gsub( "	%S+ = effector%.create%_fx%(", "" ):sub( 1, -3 )
				:gsub( "(%-?%d+[%.%d]*)f", "%1 * frame_dur" )
				:gsub( "\\\\", "\\" ):gsub( "fx%.pos_x", "$x" ):gsub( "fx%.pos_y", "$y" ):gsub( "ceil", "math.ceil" ):gsub( "floor", "math.floor" )
				:gsub( "syl%.middle", "line.middle" ):gsub( "syl%.center", "line.left + syl.center" ):gsub( "fx%.dur", "line.duration" )--:gsub(  )
				:gsub( "(\\fsc[xy]*)r(%d+[%.%d]*)",
					function( tag, val )
						if tag:len( ) == 5 then
							return tag .. tonumber( val ) * 100
						end
						return	"\\fscx" .. tonumber( val ) * 100 .. "\\\\fscy" .. tonumber( val ) * 100
					end
				)
				local auto4_function = loadstring( format( "return function( ) return { %s } end", auto4_tbl ) )( )
				auto4_tbl = auto4_function( )
				for k, v in pairs( auto4_tbl ) do
					if k <= 3 then
						auto4_tbl[ k ] = nil
					end
					if v == "" or v == true or v == false or v == "Lua" then
						auto4_tbl[ k ] = nil
					end
					if k >= 6 and k <= 11 then
						auto4_tbl[ k ] = nil
					end
				end
				return ke4.table.view( auto4_tbl )
				--return auto4_tbl
			end, --!_G.ke4.file.to_auto4( )!
			
			get_lines_ass = function( File_ass )
				--_G.ke4.file.get_lines_ass( "Effector-3.6-test.ass" )
				local ass_lines = ke4.file.get_lines( File_ass, "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+%b,,%S+[ %S]*" )
				local ass_styles = ke4.file.get_lines( File_ass, "Style: %w+[%-%w ]*," )
				local count_ini = 0
				for i = 1, #ass_lines do
					if ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+(%b,,)%S+[ %S]*" ):match( "template" ) == nil
						and ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+(%b,,)%S+[ %S]*" ):match( "code" ) == nil then
						count_ini = i
						break
					end
				end
				local ass_heads = ke4.file.get_lines( File_ass, { "%[Aegisub Project Garbage%]", count_ini } )
				count_ini = #ass_heads
				local linefx, stylefx = { }, { }
				for i = 1, #ass_styles do
					stylefx[ ass_styles[ i ]:match( "Style: (%w+[%-%w ]*)," ) ] = {
						[ "raw" ] =			ass_styles[ i ],
						[ "class" ] =		string.lower( ass_styles[ i ]:match( "(%w+): %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "name" ] =		ass_styles[ i ]:match( "%w+: (%w+[%-%w ]*),%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ),
						[ "fontname" ] =	ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,(%w+[%-%w ]*),%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ),
						[ "fontsize" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,(%d+),&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "color1" ] =		ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,(&H%x+),&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) .. "&",
						[ "color2" ] =		ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,(&H%x+),&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) .. "&",
						[ "color3" ] =		ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,(&H%x+),&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) .. "&",
						[ "color4" ] =		ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,(&H%x+),%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) .. "&",
						[ "bold" ] =		(ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,(%d+),%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) == "0") and false or true,
						[ "italic" ] =		(ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,(%d+),%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) == "0") and false or true,
						[ "underline" ] =	(ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,(%d+),%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) == "0") and false or true,
						[ "strikeout" ] =	(ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,(%d+),%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) == "0") and false or true,
						[ "scale_x" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,(%d+[%.%d]*),%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "scale_y" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,(%d+[%.%d]*),%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "spacing" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,(%d+[%.%d]*),%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "angle" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,(%d+[%.%d]*),%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "borderstyle" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,(%d+[%.%d]*),%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "outline" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,(%d+[%.%d]*),%d+[%.%d]*,%d+,%d+,%d+,%d+,%d+" ) ),
						[ "shadow" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,(%d+[%.%d]*),%d+,%d+,%d+,%d+,%d+" ) ),
						[ "align" ] =		tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,(%d+),%d+,%d+,%d+,%d+" ) ),
						[ "margin_l" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,(%d+),%d+,%d+,%d+" ) ),
						[ "margin_r" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,(%d+),%d+,%d+" ) ),
						[ "margin_v" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,(%d+),%d+" ) ),
						[ "margin_b" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,(%d+),%d+" ) ),
						[ "margin_t" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,(%d+),%d+" ) ),
						[ "encoding" ] =	tonumber( ass_styles[ i ]:match( "%w+: %w+[%-%w ]*,%w+[%-%w ]*,%d+,&H%x+,&H%x+,&H%x+,&H%x+,%d+,%d+,%d+,%d+,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+[%.%d]*,%d+,%d+,%d+,%d+,(%d+)" ) )
					}
				end
				local xres, yres = 1280, 720
				for i = 1, #ass_lines do
					if ass_lines[ i ]:match( "PlayResX: %d+" ) then
						xres = tonumber( ass_lines[ i ]:match( "PlayResX: (%d+)" ) )
					end
					if ass_lines[ i ]:match( "PlayResY: %d+" ) then
						yres = tonumber( ass_lines[ i ]:match( "PlayResY: (%d+)" ) )
					end
					if ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+(%b,,)%S+[ %S]*" ):match( "template" ) == nil
						and ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+(%b,,)%S+[ %S]*" ):match( "code" ) == nil then
						linefx[ #linefx + 1 ] = {
							format =		ass_lines[ i ]:match( "(%w+): %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+%b,,%S+[ %S]*" ),
							layer =			tonumber( ass_lines[ i ]:match( "%w+: (%d+),%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+%b,,%S+[ %S]*" ) ),
							start_time =	ke4.time.HMS_to_ms( ass_lines[ i ]:match( "%w+: %d+,(%d:%d+:%d+%.%d+),%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+%b,,%S+[ %S]*" ) ),
							end_time =		ke4.time.HMS_to_ms( ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,(%d:%d+:%d+%.%d+),%w+[%w ]*%b,,%d+,%d+,%d+%b,,%S+[ %S]*" ) ),
							style =			ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,(%w+[%w ]*)%b,,%d+,%d+,%d+%b,,%S+[ %S]*" ),
							actor =			ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*(%b,,)%d+,%d+,%d+%b,,%S+[ %S]*" ):sub( 2, -2 ),
							marginl =		tonumber( ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,(%d+),%d+,%d+%b,,%S+[ %S]*" ) ),
							marginr =		tonumber( ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,(%d+),%d+%b,,%S+[ %S]*" ) ),
							marginv =		tonumber( ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,(%d+)%b,,%S+[ %S]*" ) ),
							effect =		ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+(%b,,)%S+[ %S]*" ):sub( 2, -2 ),
							text =			ass_lines[ i ]:match( "%w+: %d+,%d:%d+:%d+%.%d+,%d:%d+:%d+%.%d+,%w+[%w ]*%b,,%d+,%d+,%d+%b,,(%S+[ %S]*)" ),
						}
					end
					for i = 1, #linefx do
						linefx[ i ].duration		= linefx[ i ].end_time - linefx[ i ].start_time
						linefx[ i ].styleref		= stylefx[ linefx[ i ].style ]
						linefx[ i ].text_strpped	= ke4.text.remove_extra_space( ke4.text.remove_tags( linefx[ i ].text ) )
						--------------------------
						local width, height, descent, extlead = aegisub.text_extents( linefx[ i ].styleref, linefx[ i ].text:gsub( "\\N", " " ):gsub( "  ", " " ) )
						--------------------------
						linefx[ i ].width		= ke4.math.round( width, 3 )
						linefx[ i ].height		= ke4.math.round( height, 3 )
						linefx[ i ].descent		= ke4.math.round( descent, 3 )
						linefx[ i ].extlead		= ke4.math.round( extlead, 3 )
						--------------------------
						local options_lft_line = {
							[ 1 ] = linefx[ i ].styleref.margin_l,
							[ 2 ] = (xres + linefx[ i ].styleref.margin_l - linefx[ i ].styleref.margin_r) / 2 - width / 2,
							[ 3 ] = xres - linefx[ i ].styleref.margin_r - width
						}
						local options_top_line = {
							[ 1 ] = yres - linefx[ i ].styleref.margin_b - height,
							[ 2 ] = yres / 2 - height / 2,
							[ 3 ] = linefx[ i ].styleref.margin_t
						}
						--------------------------
						linefx[ i ].left		= ke4.math.round( options_lft_line[ (linefx[ i ].styleref.align - 1) % 3 + 1 ], 3 )
						linefx[ i ].center		= ke4.math.round( linefx[ i ].left + width / 2, 3 )
						linefx[ i ].right		= ke4.math.round( linefx[ i ].left + width, 3 )
						linefx[ i ].top			= ke4.math.round( options_top_line[ ceil( linefx[ i ].styleref.align / 3 ) ], 3 )
						linefx[ i ].middle		= ke4.math.round( linefx[ i ].top + height / 2, 3 )
						linefx[ i ].bottom		= ke4.math.round( linefx[ i ].top + height, 3 )
						-- linefx[ i ].pretime
						if i == 1 
							or linefx[ i - 1 ].style ~= linefx[ i ].style then
							linefx[ i ].pretime = 0
						else
							linefx[ i ].pretime = linefx[ i ].start_time - linefx[ i - 1 ].end_time
						end
						-- linefx[ i ].posttime
						if i == #linefx
							or linefx[ i + 1 ].style ~= linefx[ i ].style then
							linefx[ i ].posttime = 0
						else
							linefx[ i ].posttime = linefx[ i + 1 ].start_time - linefx[ i ].end_time
						end
						------------------------
						local function tags_in_word( Word )
							-- retorna los tags de cada word
							local tags_word_tbl = { }
							for cap in Word:gmatch( "%b{}" ) do
								table.insert( tags_word_tbl, cap:sub( 2, -2 ) )
							end
							local word_tags_str = table.concat( tags_word_tbl )
							return "{" .. word_tags_str .. "}"
						end
						------------------------
						--▼ word subtable ------
						local words_line, words_dur	= ke4.text.text2word( linefx[ i ].text, linefx[ i ].duration )
						linefx[ i ].word		= { }
						linefx[ i ].word.n		= #words_line
						linefx[ i ].word.text	= ""
						local words_left		= linefx[ i ].left
						local words_start		= 0
						for k = 1, #words_line do
							linefx[ i ].word.i					= k
							linefx[ i ].word[ k ]				= { }
							linefx[ i ].word[ k ].text			= ke4.text.karaoke_true( words_line )
																and words_line[ k ]:gsub( "KEclip", " " )
																or format( "{\\k%s}%s", ke4.math.round( words_dur[ k ] / 10 ), words_line[ k ] ):gsub( "KEclip", " " )
							linefx[ i ].word[ k ].tags			= tags_in_word( linefx[ i ].word[ k ].text )
							linefx[ i ].word[ k ].text_raw		= linefx[ i ].word[ k ].text:gsub( "KEclip", " " )
							linefx[ i ].word[ k ].text_stripped	= ke4.text.text2stripped( words_line[ k ] )
							linefx[ i ].word[ k ].text1			= ke4.text.remove_tags( words_line[ k ] ):gsub( "KEfx", "" )
							linefx[ i ].word[ k ].text2			= linefx[ i ].word[ k ].text_stripped:gsub( "KEfx", "" )
							linefx[ i ].word[ k ].width_t		= aegisub.text_extents( linefx[ i ].styleref, linefx[ i ].word[ k ].text1 )
							linefx[ i ].word[ k ].width			= aegisub.text_extents( linefx[ i ].styleref, linefx[ i ].word[ k ].text2 )
							linefx[ i ].word[ k ].left			= words_left
							linefx[ i ].word[ k ].center		= words_left + linefx[ i ].word[ k ].width / 2
							linefx[ i ].word[ k ].right			= words_left + linefx[ i ].word[ k ].width
							linefx[ i ].word[ k ].top			= linefx[ i ].top
							linefx[ i ].word[ k ].middle		= linefx[ i ].middle
							linefx[ i ].word[ k ].bottom		= linefx[ i ].bottom
							linefx[ i ].word[ k ].height		= linefx[ i ].height
							linefx[ i ].word[ k ].duration		= words_dur[ k ]
							linefx[ i ].word[ k ].dur			= words_dur[ k ]
							linefx[ i ].word[ k ].start_time	= words_start
							linefx[ i ].word[ k ].end_time		= linefx[ i ].word[ k ].start_time + linefx[ i ].word[ k ].duration
							linefx[ i ].word[ k ].mid_time		= linefx[ i ].word[ k ].start_time + linefx[ i ].word[ k ].duration / 2
							linefx[ i ].word.text				= linefx[ i ].word.text .. linefx[ i ].word[ k ].text:gsub( "KEfx", "" )
							words_left 							= words_left  + linefx[ i ].word[ k ].width_t
							words_start							= words_start + linefx[ i ].word[ k ].duration
						end
						------------------------
						--▼ syl subtable -------
						local syls_line, syls_dur = ke4.text.text2syl( linefx[ i ].text, linefx[ i ].duration )
						linefx[ i ].syl			= { }
						linefx[ i ].syl.n		= #syls_line
						linefx[ i ].syl.text	= ""
						local syls_left			= linefx[ i ].left
						local syls_start		= 0
						for k = 1, #syls_line do
							linefx[ i ].syl.i					= k
							linefx[ i ].syl[ k ]				= { }
							linefx[ i ].syl[ k ].text			= ke4.text.karaoke_true( syls_line )
																and syls_line[ k ]:gsub( "KEclip", " " )
																or format( "{\\k%s}%s", ke4.math.round( syls_dur[ k ] / 10 ), syls_line[ k ] ):gsub( "KEclip", " " )
							linefx[ i ].syl[ k ].tags			= linefx[ i ].syl[ k ].text:match( "%b{}" )
							linefx[ i ].syl[ k ].text_raw		= linefx[ i ].syl[ k ].text:gsub( "KEclip", " " )
							linefx[ i ].syl[ k ].text_stripped	= ke4.text.text2stripped( syls_line[ k ] )
							linefx[ i ].syl[ k ].text1			= ke4.text.remove_tags( syls_line[ k ] ):gsub( "KEfx", "" )
							linefx[ i ].syl[ k ].text2			= linefx[ i ].syl[ k ].text_stripped:gsub( "KEfx", "" )
							linefx[ i ].syl[ k ].width_t		= aegisub.text_extents( linefx[ i ].styleref, linefx[ i ].syl[ k ].text1 )
							linefx[ i ].syl[ k ].width			= aegisub.text_extents( linefx[ i ].styleref, linefx[ i ].syl[ k ].text2 )
							linefx[ i ].syl[ k ].left			= syls_left
							linefx[ i ].syl[ k ].center			= syls_left + linefx[ i ].syl[ k ].width / 2
							linefx[ i ].syl[ k ].right			= syls_left + linefx[ i ].syl[ k ].width
							linefx[ i ].syl[ k ].top			= linefx[ i ].top
							linefx[ i ].syl[ k ].middle			= linefx[ i ].middle
							linefx[ i ].syl[ k ].bottom			= linefx[ i ].bottom
							linefx[ i ].syl[ k ].height			= linefx[ i ].height
							linefx[ i ].syl[ k ].duration		= syls_dur[ k ]
							linefx[ i ].syl[ k ].dur			= syls_dur[ k ]
							linefx[ i ].syl[ k ].start_time		= syls_start
							linefx[ i ].syl[ k ].end_time		= linefx[ i ].syl[ k ].start_time + linefx[ i ].syl[ k ].duration
							linefx[ i ].syl[ k ].mid_time		= linefx[ i ].syl[ k ].start_time + linefx[ i ].syl[ k ].duration / 2
							linefx[ i ].syl.text				= linefx[ i ].syl.text .. linefx[ i ].syl[ k ].text:gsub( "KEfx", "" )
							syls_left 							= syls_left  + linefx[ i ].syl[ k ].width_t
							syls_start							= syls_start + linefx[ i ].syl[ k ].duration
						end
					end
				end
				--return Line_i - count_ini + 7
				return ke4.table.view( linefx )
				--return ke4.table.view( stylefx )
			end,
			
			get_lines = function( File_lua, Number_or_match )
				--retorna una tabla con las líneas seleccionadas de un archivo
				local File_lua = File_lua or "my.lua"
				local File = io.open( File_lua, "r" )
				--Number_or_match = number, { ini, fin }, {{ n1, n2, n3 ... }}, match
				local n = Number_or_match or 1
				local Lines_tbl = { }
				local count = 1
				local is_match = false
				for line in File:lines( ) do
					if n == "r" then
						Lines_tbl[ #Lines_tbl + 1 ] = line
					elseif type( n ) == "number" then --number
						if count == n then
							Lines_tbl[ #Lines_tbl + 1 ] = line
						end
						count = count + 1
					elseif type( n ) == "table"
						and type( n[ 1 ] ) == "table" then --lines
						if ke4.table.inside( n[ 1 ], count ) then
							Lines_tbl[ #Lines_tbl + 1 ] = line
						end
						count = count + 1
					elseif type( n ) == "table" then --ini & fin
						if type( n[ 1 ] ) == "number"
							and type( n[ 2 ] ) == "number" then --number & number
							if count >= n[ 1 ]
								and count <= n[ 2 ] then
								Lines_tbl[ #Lines_tbl + 1 ] = line
							end
						elseif type( n[ 1 ] ) == "number"
							and type( n[ 2 ] ) == "string" then --number & match
							if count >= n[ 1 ] then
								Lines_tbl[ #Lines_tbl + 1 ] = line
								if line:match( n[ 2 ] ) then
									break
								end
							end
						elseif type( n[ 1 ] ) == "string"
							and type( n[ 2 ] ) == "number" then --match & number
							if line:match( n[ 1 ] ) then
								is_match = true
							end --!_G.ke4.table.view( _G.ke4.file.get_lines( "unname.ass", { "%[Aegisub Project Garbage%]", 15 } ) )!
							if is_match == true then
								Lines_tbl[ #Lines_tbl + 1 ] = line
								if count == n[ 2 ] then
									break
								end
							end
						elseif type( n[ 1 ] ) == "string"
							and type( n[ 2 ] ) == "string" then --match & match
							if line:match( n[ 1 ] ) then
								is_match = true
							end
							if is_match == true then
								Lines_tbl[ #Lines_tbl + 1 ] = line
								if line:match( n[ 2 ] ) then
									break
								end
							end
						end
						count = count + 1
					elseif type( n ) == "string" then --match
						if line:match( n ) then
							Lines_tbl[ #Lines_tbl + 1 ] = line
						end
					end
				end
				File:close( )
				return Lines_tbl
			end,
			
			gsub = function( File_lua, ... )
				--modifica un archivo seleccionado usando string.gsub en sus líneas :D
				local File_lua = File_lua or "my.lua"
				local File = io.open( File_lua, "r" )		-- abre el archivo
				local Content_file = File:read( "*all" )	-- copia su contenido en una variable
				File:close( )								-- cierra el archivo
				local Modifx = { ... }						-- tabla de modificaciones
				if type( Modifx[ 1 ][ 1 ] ) == "table" then
					Modifx = ...
				end -- ... = { match1, rep1 }, { match2, rep2 } or {{ match1, rep1 }, { match2, rep2 }}
				-- modificacoines específicas del contenido del archivo ------------------
				for i = 1, #Modifx do
					Content_file = Content_file:gsub( Modifx[ i ][ 1 ], Modifx[ i ][ 2 ] )
				end
				--------------------------------------------------------------------------
				File = io.open( File_lua, "w+" )			-- reabre el archivo y elimina su contenido
				File:write( Content_file )					-- escribe las modificaciones en el archivo
				File:close( )								-- cierra el archivo
				return "" --{ "%-%-%[%[%w+[ %w%/%:]*%]%] ", "" }
			end,
			
			match = function( File_lua, Match_or_tbl )
				--busca coincidencias en un archivo y retorna true o false
				local File_lua = File_lua or "my.lua"
				local File = io.open( File_lua, "r" )		-- abre el archivo
				local Content_file = File:read( "*all" )	-- copia su contenido en una variable
				File:close( )								-- cierra el archivo
				local Match_or_tbl = Match_or_tbl or "fxKE"	-- coincidencias
				if type( Match_or_tbl ) == "table" then
					for i = 1, #Match_or_tbl do
						if Content_file:match( Match_or_tbl[ i ] ) then
							return true
						end
					end
					return false
				end
				if Content_file:match( Match_or_tbl ) then
					return true
				end
				return false
			end,
			
			gmatch = function( File_lua, Match_or_tbl )
				--busca coincidencias en un archivo y retorna una tabla con ellas
				local File_lua = File_lua or "my.lua"
				local File = io.open( File_lua, "r" )		-- abre el archivo
				local Content_file = File:read( "*all" )	-- copia su contenido en una variable
				File:close( )								-- cierra el archivo
				local Match_or_tbl = Match_or_tbl or "fxKE"	-- coincidencias
				local Gmatch_tbl = { }
				if type( Match_or_tbl ) == "table" then
					for i = 1, #Match_or_tbl do
						Gmatch_tbl[ i ] = { }
						for cap in Content_file:gmatch( Match_or_tbl[ i ] ) do
							Gmatch_tbl[ i ][ #Gmatch_tbl[ i ] + 1 ] = cap
						end
					end
				else
					for cap in Content_file:gmatch( Match_or_tbl ) do
						Gmatch_tbl[ #Gmatch_tbl + 1 ] = cap
					end
				end
				return Gmatch_tbl
			end,
			
			count = function( File_lua, Match_or_tbl )
				--retorna la cantidad de veces que una coincidencia está en un archivo
				local File_lua = File_lua or "my.lua"
				local File = io.open( File_lua, "r" )		-- abre el archivo
				local Content_file = File:read( "*all" )	-- copia su contenido en una variable
				File:close( )								-- cierra el archivo
				local Match_or_tbl = Match_or_tbl or "fxKE"	-- coincidencias
				local count_fl = 0
				if type( Match_or_tbl ) == "table" then
					for i = 1, #Match_or_tbl do
						for cap in Content_file:gmatch( Match_or_tbl[ i ] ) do
							count_fl = count_fl + 1
						end
					end
				else
					for cap in Content_file:gmatch( Match_or_tbl ) do
						count_fl = count_fl + 1
					end
				end
				return count_fl
			end
		},
		
		-- Image sublibrary
		image = {
			data = function( bmp_image )
				local bmp_color, bmp_alpha = { }, { }
				local bmp_image = bmp_image or "test.bmp"
				local bmp_xdata = ke4.decode.create_bmp_reader( bmp_image ).data_packed( )
				for i = 1, #bmp_xdata do
					bmp_color[ i ] = ke4.color.val2ass( bmp_xdata[ i ].r, bmp_xdata[ i ].g, bmp_xdata[ i ].b )
					bmp_alpha[ i ] = ke4.alpha.val2ass( 255 - bmp_xdata[ i ].a )
				end
				return bmp_color, bmp_alpha
			end, --image.data( )
			
			to_pixels = function( bmp_image, Size, Return_pos )
				local bmp_image = bmp_image or "test.bmp"
				local Size = Size or 1
				local bmp_color, bmp_alpha = ke4.image.data( bmp_image )
				local bmp_wth = ke4.decode.create_bmp_reader( bmp_image ).width( )
				local bmp_hht = ke4.decode.create_bmp_reader( bmp_image ).height( )
				local pixels, pixel_pos = { }, { }
				for i = 1, #bmp_color do
					if bmp_alpha[ i ] ~= "&HFF&" then
						pixels[ #pixels + 1 ] = {
							x = ((i - 1) % bmp_wth + 1) * Size - bmp_wth * Size / 2 + Size / 2,
							y = ceil( i / bmp_hht - 1 ) * Size - bmp_hht * Size / 2 + Size / 2,
							a = bmp_alpha[ i ],
							c = bmp_color[ i ]
						}
						pixel_pos[ #pixel_pos + 1 ] = {
							x = ke4.math.round( ((i - 1) % bmp_wth + 1) * Size - bmp_wth * Size / 2 + Size / 2 ),
							y = ke4.math.round( ceil( i / bmp_hht - 1 ) * Size - bmp_hht * Size / 2 + Size / 2 )
						}
					end
				end
				if Return_pos then
					return pixel_pos, pixels
				end
				return pixels
				--code once: pixels = _G.ke4.image.to_pixels( "auto.bmp" )
				--!maxloop( #pixels )!{\an5\pos(!$x + pixels[ j ].x!,!$y + pixels[ j ].y!)\1c!pixels[ j ].c!\1a!pixels[ j ].a!\bord0\shad0\p1}m 0 0 l 0 1 l 1 1 l 1 0 
			end,
		},
		
		-- Decoder sublibrary
		decode = {
			create_bmp_reader = function( filename )
				-- Creates BMP file reader
				if type( filename ) ~= "string" then
					error( "bitmap filename expected", 2 )
				end
				local function bmp_decode( filename )
					local file = io.open( filename, "rb" )
					if file then
						local header = file:read( 14 )
						if not header or #header ~= 14 then
							return "couldn't read file header"
						end
						if header:sub( 1, 2 ) == "BM" then
							local file_size, data_offset = bton( header:sub( 3, 6 ) ), bton( header:sub( 11, 14 ) )
							header = file:read( 24 )
							if not header or #header ~= 24 then
								return "couldn't read DIB header"
							end
							local width, height, planes, bit_depth, compression, data_size = bton( header:sub( 5, 8 ) ), bton( header:sub( 9, 12 ) ), bton( header:sub( 13, 14 ) ), bton( header:sub( 15, 16 ) ), bton( header:sub( 17, 20 ) ), bton( header:sub( 21, 24 ) )
							if width >= 2 ^ 31 then
								return "pixels in right-to-left order are not supported"
							elseif planes ~= 1 then
								return "planes must be 1"
							elseif bit_depth ~= 24 and bit_depth ~= 32 then
								return "bit depth must be 24 or 32"
							elseif compression ~= 0 then
								return "must be uncompressed RGB"
							elseif data_size == 0 then
								return "data size must not be zero"
							end
							if height >= 2 ^ 31 then
								height = height - 2 ^ 32
							end
							file:seek( "set", data_offset )
							local data = file:read( data_size )
							if not data or #data ~= data_size then
								return "not enough data"
							end
							local row_size = floor( (bit_depth * width + 31) / 32 ) * 4
							file:close( )
							return file_size, width, height, bit_depth, data_size, data, row_size
						end
					end
				end
				
				local function png_decode( filename )
					-- PNG decode library available?
					if libpng then
						local file = io.open( filename, "rb" )
						if file then
							local file_content = file:read( "*a" )
							file:close( )
							local file_size = #file_content
							if file_size > ffix.C.PNG_SIGNATURE_SIZE and libpng.png_sig_cmp( ffix.cast( "png_const_bytep", file_content ), 0, ffix.C.PNG_SIGNATURE_SIZE ) == 0 then
								local ppng, pinfo, err = ffix.new( "png_structp[1]" ), ffix.new( "png_infop[1]" )
								local function err_func( png, message )
									libpng.png_destroy_read_struct( ppng, pinfo, nil )
									err = ffix.string( message )
								end
								ppng[ 0 ] = libpng.png_create_read_struct( ffix.cast( "char*", "1.5.14" ), nil, err_func, err_func )
								if not ppng[ 0 ] then
									return "couldn't create png read structure"
								end
								pinfo[ 0 ] = libpng.png_create_info_struct( ppng[ 0 ] )
								if not pinfo[ 0 ] then
									libpng.png_destroy_read_struct( ppng, nil, nil )
									return "couldn't create png info structure"
								end
								local file_pos, file_content_bytes = 0, ffix.cast( "png_bytep", file_content )
								libpng.png_set_read_fn( ppng[ 0 ], nil, function( png, output_bytes, required_bytes )
									if file_pos + required_bytes <= file_size then
										ffix.C.memcpy( output_bytes, file_content_bytes+file_pos, required_bytes )
										file_pos = file_pos + required_bytes
									end
								end )
								libpng.png_read_png( ppng[ 0 ], pinfo[ 0 ], ffix.C.PNG_TRANSFORM_STRIP_16 + ffix.C.PNG_TRANSFORM_PACKING + ffix.C.PNG_TRANSFORM_EXPAND + ffix.C.PNG_TRANSFORM_BGR, nil )
								if err then
									return err
								end
								libpng.png_set_interlace_handling( ppng[ 0 ] )
								libpng.png_read_update_info( ppng[ 0 ], pinfo[ 0 ] )
								if err then
									return err
								end
								local width, height, color_type, row_size = libpng.png_get_image_width( ppng[ 0 ], pinfo[ 0 ] ), libpng.png_get_image_height( ppng[ 0 ], pinfo[ 0 ] ), libpng.png_get_color_type( ppng[ 0 ], pinfo[ 0 ] ), libpng.png_get_rowbytes( ppng[ 0 ], pinfo[ 0 ] )
								local data_size, bit_depth = height * row_size
								if color_type == ffix.C.PNG_COLOR_TYPE_RGB then
									bit_depth = 24
								elseif color_type == ffix.C.PNG_COLOR_TYPE_RGBA then
									bit_depth = 32
								else
									libpng.png_destroy_read_struct( ppng, pinfo, nil )
									return "png data conversion to BGR(A) colorspace failed"
								end
								local rows = libpng.png_get_rows( ppng[ 0 ], pinfo[ 0 ] )
								local data, data_n = { }, 0
								for i = 0, height - 1 do
									data_n = data_n + 1
									data[ data_n ] = ffix.string( rows[ i ], row_size )
								end
								data = table.concat( data )
								libpng.png_destroy_read_struct( ppng, pinfo, nil )
								return file_size, width, height, bit_depth, data_size, data, row_size
							end
						end
					end
				end
				local bottom_up
				local file_size, width, height, bit_depth, data_size, data, row_size = bmp_decode( filename )
				if not file_size then
					file_size, width, height, bit_depth, data_size, data, row_size = png_decode( filename )
					if not file_size then
						error( "couldn't decode file", 2 )
					elseif type( file_size ) == "string" then
						error( file_size, 2 )
					else
						bottom_up = false
					end
				elseif type( file_size ) == "string" then
					error( file_size, 2 )
				else
					bottom_up = height >= 0
					height = abs( height )
				end
				local obj
				obj = {
					file_size = function( )
						return file_size
					end,
					width = function( )
						return width
					end,
					height = function( )
						return height
					end,
					bit_depth = function( )
						return bit_depth
					end,
					data_size = function( )
						return data_size
					end,
					row_size = function( )
						return row_size
					end,
					bottom_up = function( )
						return bottom_up
					end,
					data_raw = function( )
						return data
					end,
					data_packed = function( )
						local data_packed, data_packed_n = { }, 0
						local first_row, last_row, row_step
						if bottom_up then
							first_row, last_row, row_step = height - 1, 0, -1
						else
							first_row, last_row, row_step = 0, height - 1, 1
						end
						if bit_depth == 24 then
							local last_row_item, r, g, b = (width - 1) * 3
							for y = first_row, last_row, row_step do
								y = 1 + y * row_size
								for x=0, last_row_item, 3 do
									b, g, r = data:byte( y + x, y + x + 2 )
									data_packed_n = data_packed_n + 1
									data_packed[ data_packed_n ] = {
										r = r,
										g = g,
										b = b,
										a = 255
									}
								end
							end
						else
							local last_row_item, r, g, b, a = (width - 1) * 4
							for y = first_row, last_row, row_step do
								y = 1 + y * row_size
								for x = 0, last_row_item, 4 do
									b, g, r, a = data:byte( y + x, y + x + 3 )
									data_packed_n = data_packed_n + 1
									data_packed[ data_packed_n ] = {
										r = r,
										g = g,
										b = b,
										a = a
									}
								end
							end
						end
						return data_packed
					end,
					data_text = function( )
						local data_pack, text, text_n = obj.data_packed( ), { "{\\bord0\\shad0\\an7\\p1}" }, 1
						local x, y, off_x, chunk_size, color1, color2 = 0, 0, 0
						local i, n = 1, #data_pack
						while i <= n do
							if x == width then
								x = 0
								y = y + 1
								off_x = off_x - width
							end
							chunk_size, color1, text_n = 1, data_pack[ i ], text_n + 1
							if color1.a == 0 then
								for xx = x + 1, width - 1 do
									color2 = data_pack[ i +(xx - x) ]
									if not (color2 and color2.a == 0) then
										break
									end
									chunk_size = chunk_size + 1
								end
								text[ text_n ] = format( "{}m %d %d l %d %d", off_x, y, off_x + chunk_size, y + 1 )
							else
								for xx = x + 1, width - 1 do
									color2 = data_pack[ i + (xx - x) ]
									if not (color2 and color1.r == color2.r and color1.g == color2.g and color1.b == color2.b and color1.a == color2.a) then
										break
									end
									chunk_size = chunk_size + 1
								end
								text[ text_n ] = format( "{\\c&H%02X%02X%02X&\\1a&H%02X&}m %d %d l %d %d %d %d %d %d",
																		color1.b, color1.g, color1.r, 255 - color1.a, off_x, y, off_x + chunk_size, y, off_x + chunk_size, y + 1, off_x, y + 1 )
							end
							i, x = i + chunk_size, x + chunk_size
						end
						return table.concat( text )
					end
				}
				return obj
			end,
			
			create_wav_reader = function( filename )
				-- Create WAV file reader
				if type( filename ) ~= "string" then
					error( "audio filename expected", 2 )
				end
				local file = io.open( filename, "rb" )
				if not file then
					error( "couldn't open file", 2 )
				end
				local header = file:read( 12 )
				if not header or #header ~= 12 then
					error( "couldn't read file header", 2 )
				elseif header:sub( 1, 4 ) ~= "RIFF" or header:sub( 9, 12 ) ~= "WAVE" then
					error( "not a wave file", 2 )
				end
				local file_size, channels_number, sample_rate, byte_rate, block_align, bits_per_sample = bton( header:sub( 5, 8 ) ) + 8
				local data_begin, data_end
				local chunk_type, chunk_size
				while true do
					chunk_type, chunk_size = file:read( 4 ), file:read( 4 )
					if not chunk_size or #chunk_size ~= 4 then
						break
					end
					chunk_size = bton( chunk_size )
					if chunk_type == "fmt " then
						header = file:read( 16 )
						if chunk_size < 16 or not header or #header ~= 16 then
							error( "format chunk corrupted", 2 )
						elseif bton( header:sub( 1, 2 ) ) ~= 1 then
							error( "data must be in PCM format", 2 )
						end
						channels_number, sample_rate, byte_rate, block_align, bits_per_sample = bton( header:sub( 3, 4 ) ), bton( header:sub( 5, 8 ) ), bton( header:sub( 9, 12 ) ), bton( header:sub( 13, 14 ) ), bton( header:sub( 15, 16 ) )
						if bits_per_sample ~= 8 and bits_per_sample ~= 16 and bits_per_sample ~= 24 and bits_per_sample ~= 32 then
							error( "bits per sample must be 8, 16, 24 or 32", 2 )
						elseif channels_number == 0 or sample_rate == 0 or byte_rate == 0 or block_align == 0 then
							error( "invalid format data", 2 )
						end
						file:seek( "cur", chunk_size - 16 )
					elseif chunk_type == "data" then
						data_begin = file:seek( )
						data_end = data_begin + chunk_size
						file:seek( "cur", chunk_size )
					else
						file:seek( "cur", chunk_size )
					end
				end
				if not bits_per_sample or not data_end then
					error( "format or data are missing", 2 )
				end
				local samples_per_channel = (data_end - data_begin) / block_align
				file:seek( "set", data_begin )
				local obj
				obj = {
					file_size = function( )
						return file_size
					end,
					channels_number = function( )
						return channels_number
					end,
					sample_rate = function( )
						return sample_rate
					end,
					byte_rate = function( )
						return byte_rate
					end,
					block_align = function( )
						return block_align
					end,
					bits_per_sample = function( )
						return bits_per_sample
					end,
					samples_per_channel = function( )
						return samples_per_channel
					end,
					min_max_amplitude = function( )
						local half_level = 2 ^ bits_per_sample / 2
						return -half_level, half_level - 1
					end,
					sample_from_ms = function( ms )
						if type( ms ) ~= "number" or ms < 0 then
							error( "positive number expected", 2 )
						end
						return ms * 0.001 * sample_rate
					end,
					ms_from_sample = function( sample )
						if type( sample ) ~= "number" or sample < 0 then
							error( "positive number expected", 2 )
						end
						return sample / sample_rate * 1000
					end,
					position = function( pos )
						if pos ~= nil and (type( pos ) ~= "number" or pos < 0) then
							error( "optional positive number expected", 2 )
						elseif pos then
							file:seek( "set", data_begin + pos * block_align )
						end
						return (file:seek( ) - data_begin) / block_align
					end,
					samples_interlaced = function( n )
						if type( n ) ~= "number" or floor( n ) < 1 then
							error( "positive number greater-equal one expected", 2 )
						end
						local output, bytes = { n = 0 }, file:read( floor( n ) * block_align )
						if bytes then
							local bytes_per_sample, sample = bits_per_sample / 8
							local max_amplitude, amplitude_fix = ( { 127, 32767, 8388607, 2147483647 } )[ bytes_per_sample ], ( { 256, 65536, 16777216, 4294967296 } )[ bytes_per_sample ]
							for i = 1, #bytes, bytes_per_sample do
								sample = bton( bytes:sub( i, i + bytes_per_sample - 1 ) )
								output.n = output.n + 1
								output[ output.n ] = sample > max_amplitude and sample - amplitude_fix or sample
							end
						end
						return output
					end,
					samples = function( n )
						local success, samples = pcall( obj.samples_interlaced, n )
						if not success then
							error( samples, 2 )
						end
						local output, channel_samples = { n = channels_number }
						for c = 1, output.n do
							channel_samples = { n = floor( samples.n / channels_number ) }
							for s = 1, channel_samples.n do
								channel_samples[ s ] = samples[ c + (s - 1) * channels_number ]
							end
							output[ c ] = channel_samples
						end
						return output
					end
				}
				return obj
			end,
			
			create_frequency_analyzer = function( samples, sample_rate )
				if type( samples ) ~= "table" or type( sample_rate ) ~= "number" or sample_rate < 2 or sample_rate % 2 ~= 0 then
					error( "samples table and sample rate expected", 2 )
				end
				local samples_n = #samples
				if samples_n < 2 then
					error( "not enough samples", 2 )
				end
				local sample
				for i = 1, samples_n do
					sample = samples[ i ]
					if type( sample ) ~= "number" then
						error( "samples have to be numbers", 2 )
					elseif sample < -1 or sample > 1 then
						error( "samples have to be in range -1 <> 1", 2 )
					end
				end
				samples_n = 2 ^ floor( log( samples_n, 2 ) )
				local complex_t
				do
					local complex = { }
					complex_t = function( r, i )
						return setmetatable( { r = r, i = i }, complex )
					end
					local function tocomplex( a, b )
						if getmetatable( a ) ~= complex then return { r = a, i = 0 }, b
						elseif getmetatable( b ) ~= complex then return a, { r = b, i = 0 }
						else return a, b end
					end
					complex.__add = function( a, b )
						local c1, c2 = tocomplex( a, b )
						return complex_t( c1.r + c2.r, c1.i + c2.i )
					end
					complex.__sub = function( a, b )
						local c1, c2 = tocomplex( a, b )
						return complex_t( c1.r - c2.r, c1.i - c2.i )
					end
					complex.__mul = function( a, b )
						local c1, c2 = tocomplex( a, b )
						return complex_t( c1.r * c2.r - c1.i * c2.i, c1.r * c2.i + c1.i * c2.r )
					end
				end
				local function polar( theta )
					return complex_t( cos( theta ), sin( theta ) )
				end
				local function magnitude( c )
					return math.sqrt( c.r ^ 2 + c.i ^ 2 )
				end
				local function fft( x )
					local N = x.n
					if N > 1 then
						local even, odd = { n = 0 }, { n = 0 }
						for i = 1, N, 2 do
							even.n = even.n + 1
							even[ even.n ] = x[ i ]
						end
						for i = 2, N, 2 do
							odd.n = odd.n + 1
							odd[ odd.n ] = x[ i ]
						end
						fft( even )
						fft( odd )
						local t
						for k = 1, N / 2 do
							t = polar( -2 * pi * (k - 1) / N ) * odd[ k ]
							x[ k ] = even[ k ] + t
							x[ k + N / 2 ] = even[ k ] - t
						end
					end
				end
				local data = { n = samples_n }
				for i = 1, data.n do
					data[ i ] = complex_t( samples[ i ], 0 )
				end
				fft( data )
				for i = 1, data.n do
					data[ i ] = magnitude( data[ i ] )
				end
				local frequencies, frequency_sum, sample_rate_half = { n = data.n / 2 }, 0, sample_rate / 2
				for i = 1, frequencies.n do
					frequency_sum = frequency_sum + data[ i ]
				end
				if frequency_sum == 0 then
					frequencies[ 1 ] = { freq = 0, weight = 1 }
					for i = 2, frequencies.n do
						frequencies[ i ] = { freq = (i - 1) / (frequencies.n - 1) * sample_rate_half, weight = 0 }
					end
				else
					for i = 1, frequencies.n do
						frequencies[ i ] = { freq = (i - 1) / (frequencies.n - 1) * sample_rate_half, weight = data[ i ] / frequency_sum }
					end
				end
				return {
					frequencies = function( )
						return ke4.table.copy( frequencies )
					end,
					frequency_weight = function( freq )
						if type( freq ) ~= "number" or freq < 0 or freq > sample_rate_half then
							error( "valid frequency expected", 2 )
						end
						local frequency
						for i = 1, frequencies.n do
							frequency = frequencies[ i ]
							if frequency.freq == freq then
								return frequency.weight
							elseif frequency.freq > freq then
								local frequency_last = frequencies[ i - 1 ]
								return (freq - frequency_last.freq) / (frequency.freq - frequency_last.freq) * (frequency.weight - frequency_last.weight) + frequency_last.weight
							end
						end
					end,
					frequency_range_weight = function( freq_min, freq_max )
						if type( freq_min ) ~= "number" or freq_min < 0 or freq_min > sample_rate_half or
							type( freq_max ) ~= "number" or freq_max < 0 or freq_max > sample_rate_half or
							freq_min > freq_max then
							error( "valid frequencies expected", 2 )
						end
						local weight_sum, frequency = 0
						for i = 1, frequencies.n do
							frequency = frequencies[ i ]
							if frequency.freq >= freq_min then
								if frequency.freq <= freq_max then
									weight_sum = weight_sum + frequency.weight
								else
									break
								end
							end
						end
						return weight_sum
					end
				}
			end,
			
			create_font = function( family, bold, italic, underline, strikeout, size, xscale, yscale, hspace )
				-- Creates font
				if type( family ) ~= "string" or type( bold ) ~= "boolean" or type( italic ) ~= "boolean" or type( underline ) ~= "boolean" or type( strikeout ) ~= "boolean" or type( size ) ~= "number" or size <= 0 or
					(xscale ~= nil and type( xscale ) ~= "number") or (yscale ~= nil and type( yscale ) ~= "number") or (hspace ~= nil and type( hspace ) ~= "number") then
					error( "expected family, bold, italic, underline, strikeout, size and optional horizontal & vertical scale and intercharacter space", 2 )
				end
				if not xscale then
					xscale = 1
				end
				if not yscale then
					yscale = 1
				end
				if not hspace then
					hspace = 0
				end
				local upscale = FONT_PRECISION
				local downscale = 1 / upscale
				if ffix.os == "Windows" then
					local resources_deleter
					local dc = ffix.gc( ffix.C.CreateCompatibleDC( nil ), function( ) resources_deleter( ) end )
					ffix.C.SetMapMode( dc, ffix.C.MM_TEXT2 )
					ffix.C.SetBkMode( dc, ffix.C.TRANSPARENT2 )
					family = utf8_to_utf16( family )
					if ffix.C.wcslen( family ) > 31 then
						error( "family name to long", 2 )
					end
					local font = ffix.C.CreateFontW(
						size * upscale,									-- nHeight
						0,												-- nWidth
						0,												-- nEscapement
						0,												-- nOrientation
						bold and ffix.C.FW_BOLD2 or ffix.C.FW_NORMAL2,	-- fnWeight
						italic and 1 or 0,								-- fdwItalic
						underline and 1 or 0,							-- fdwUnderline
						strikeout and 1 or 0,							-- fdwStrikeOut
						ffix.C.DEFAULT_CHARSET2,						-- fdwCharSet
						ffix.C.OUT_TT_PRECIS2,							-- fdwOutputPrecision
						ffix.C.CLIP_DEFAULT_PRECIS2,					-- fdwClipPrecision
						ffix.C.ANTIALIASED_QUALITY2,					-- fdwQuality
						ffix.C.DEFAULT_PITCH2 + ffix.C.FF_DONTCARE2,	-- fdwPitchAndFamily
						family
					)
					local old_font = ffix.C.SelectObject( dc, font )
					resources_deleter = function( )
						ffix.C.SelectObject( dc, old_font )
						ffix.C.DeleteObject( font )
						ffix.C.DeleteDC( dc )
					end
					return {
						metrics = function( )
							local metrics = ffix.new( "TEXTMETRICW[1]" )
							ffix.C.GetTextMetricsW( dc, metrics )
							return {
								height = metrics[ 0 ].tmHeight * downscale * yscale,
								ascent = metrics[ 0 ].tmAscent * downscale * yscale,
								descent = metrics[ 0 ].tmDescent * downscale * yscale,
								internal_leading = metrics[ 0 ].tmInternalLeading * downscale * yscale,
								external_leading = metrics[ 0 ].tmExternalLeading * downscale * yscale
							}
						end,
						text_extents = function( text )
							if type( text ) ~= "string" then
								error( "text expected", 2 )
							end
							text = utf8_to_utf16( text )
							local text_len = ffix.C.wcslen( text )
							local size = ffix.new( "SIZE[1]" )
							ffix.C.GetTextExtentPoint32W( dc, text, text_len, size )
							return {
								width = (size[ 0 ].cx * downscale + hspace * text_len) * xscale,
								height = size[ 0 ].cy * downscale * yscale
							}
						end,
						
						text_to_shape = function( text )
							-- Converts text to ASS shape
							if type( text ) ~= "string" then
								error( "text expected", 2 )
							end
							local shape, shape_n = { }, 0
							text = utf8_to_utf16( text )
							local text_len = ffix.C.wcslen( text )
							if text_len > 8192 then
								error( "text too long", 2 )
							end
							local char_widths
							if hspace ~= 0 then
								char_widths = ffix.new( "INT[?]", text_len )
								local size, space = ffix.new( "SIZE[1]" ), hspace * upscale
								for i = 0, text_len - 1 do
									ffix.C.GetTextExtentPoint32W( dc, text + i, 1, size )
									char_widths[ i ] = size[ 0 ].cx + space
								end
							end
							ffix.C.BeginPath( dc )
							ffix.C.ExtTextOutW( dc, 0, 0, 0x0, nil, text, text_len, char_widths )
							ffix.C.EndPath( dc )
							local points_n = ffix.C.GetPath( dc, nil, nil, 0 )
							if points_n > 0 then
								local points, types = ffix.new( "POINT[?]", points_n ), ffix.new( "BYTE[?]", points_n )
								ffix.C.GetPath( dc, points, types, points_n )
								local i, last_type, cur_type, cur_point = 0
								while i < points_n do
									cur_type, cur_point = types[ i ], points[ i ]
									if cur_type == ffix.C.PT_MOVETO2 then
										if last_type ~= ffix.C.PT_MOVETO2 then
											shape_n = shape_n + 1
											shape[ shape_n ] = "m"
											last_type = cur_type
										end
										shape[ shape_n + 1 ] = ke4.math.round( cur_point.x * downscale * xscale, FP_PRECISION )
										shape[ shape_n + 2 ] = ke4.math.round( cur_point.y * downscale * yscale, FP_PRECISION )
										shape_n = shape_n + 2
										i = i + 1
									elseif cur_type == ffix.C.PT_LINETO2 or cur_type == ( ffix.C.PT_LINETO2 + ffix.C.PT_CLOSEFIGURE2 ) then
										if last_type ~= ffix.C.PT_LINETO2 then
											shape_n = shape_n + 1
											shape[ shape_n ] = "l"
											last_type = cur_type
										end
										shape[ shape_n + 1 ] = ke4.math.round( cur_point.x * downscale * xscale, FP_PRECISION )
										shape[ shape_n + 2 ] = ke4.math.round( cur_point.y * downscale * yscale, FP_PRECISION )
										shape_n = shape_n + 2
										i = i + 1
									elseif cur_type == ffix.C.PT_BEZIERTO2 or cur_type == ( ffix.C.PT_BEZIERTO2 + ffix.C.PT_CLOSEFIGURE2 ) then
										if last_type ~= ffix.C.PT_BEZIERTO2 then
											shape_n = shape_n + 1
											shape[ shape_n ] = "b"
											last_type = cur_type
										end
										shape[ shape_n + 1 ] = ke4.math.round( cur_point.x * downscale * xscale, FP_PRECISION )
										shape[ shape_n + 2 ] = ke4.math.round( cur_point.y * downscale * yscale, FP_PRECISION )
										shape[ shape_n + 3 ] = ke4.math.round( points[ i + 1 ].x * downscale * xscale, FP_PRECISION )
										shape[ shape_n + 4 ] = ke4.math.round( points[ i + 1 ].y * downscale * yscale, FP_PRECISION )
										shape[ shape_n + 5 ] = ke4.math.round( points[ i + 2 ].x * downscale * xscale, FP_PRECISION )
										shape[ shape_n + 6 ] = ke4.math.round( points[ i + 2 ].y * downscale * yscale, FP_PRECISION )
										shape_n = shape_n + 6
										i = i + 3
									else
										i = i + 1
									end
									if cur_type % 2 == 1 then
										shape_n = shape_n + 1
										shape[ shape_n ] = "c"
									end
								end
							end
							ffix.C.AbortPath( dc )
							return table.concat( shape, " " )
						end
					}
				else
					if not pangocairo then
						error( "pangocairo library couldn't be loaded", 2 )
					end
					local surface = pangocairo.cairo_image_surface_create( ffix.C.CAIRO_FORMAT_A8X, 1, 1 )
					local context = pangocairo.cairo_create( surface )
					local layout
					layout = ffix.gc( pangocairo.pango_cairo_create_layout( context ), function( )
						pangocairo.g_object_unref( layout )
						pangocairo.cairo_destroy( context )
						pangocairo.cairo_surface_destroy( surface )
					end )
					local font_desc = ffix.gc( pangocairo.pango_font_description_new( ), pangocairo.pango_font_description_free )
					pangocairo.pango_font_description_set_family( font_desc, family )
					pangocairo.pango_font_description_set_weight( font_desc, bold and ffix.C.PANGO_WEIGHT_BOLD2 or ffix.C.PANGO_WEIGHT_NORMAL2 )
					pangocairo.pango_font_description_set_style( font_desc, italic and ffix.C.PANGO_STYLE_ITALIC or ffix.C.PANGO_STYLE_NORMAL )
					pangocairo.pango_font_description_set_absolute_size( font_desc, size * ffix.C.PANGO_SCALE2 * upscale )
					pangocairo.pango_layout_set_font_description( layout, font_desc )
					local attr = ffix.gc( pangocairo.pango_attr_list_new( ), pangocairo.pango_attr_list_unref )
					pangocairo.pango_attr_list_insert( attr, pangocairo.pango_attr_underline_new( underline and ffix.C.PANGO_UNDERLINE_SINGLE or ffix.C.PANGO_UNDERLINE_NONE ) )
					pangocairo.pango_attr_list_insert( attr, pangocairo.pango_attr_strikethrough_new( strikeout ) )
					pangocairo.pango_attr_list_insert( attr, pangocairo.pango_attr_letter_spacing_new( hspace * ffix.C.PANGO_SCALE2 * upscale ) )
					pangocairo.pango_layout_set_attributes( layout, attr )
					local fonthack_scale
					if LIBASS_FONTHACK then
						local metrics = ffix.gc( pangocairo.pango_context_get_metrics( pangocairo.pango_layout_get_context( layout ), pangocairo.pango_layout_get_font_description( layout ), nil ), pangocairo.pango_font_metrics_unref )
						fonthack_scale = size / ((pangocairo.pango_font_metrics_get_ascent( metrics ) + pangocairo.pango_font_metrics_get_descent( metrics )) / ffix.C.PANGO_SCALE2 * downscale)
					else
						fonthack_scale = 1
					end
					return {
						metrics = function( )
							local metrics = ffix.gc( pangocairo.pango_context_get_metrics( pangocairo.pango_layout_get_context( layout ), pangocairo.pango_layout_get_font_description( layout ), nil ), pangocairo.pango_font_metrics_unref )
							local ascent, descent = pangocairo.pango_font_metrics_get_ascent( metrics ) / ffix.C.PANGO_SCALE2 * downscale,
													pangocairo.pango_font_metrics_get_descent( metrics ) / ffix.C.PANGO_SCALE2 * downscale
							return {
								height = (ascent + descent) * yscale * fonthack_scale,
								ascent = ascent * yscale * fonthack_scale,
								descent = descent * yscale * fonthack_scale,
								internal_leading = 0,
								external_leading = pangocairo.pango_layout_get_spacing( layout ) / ffix.C.PANGO_SCALE2 * downscale * yscale * fonthack_scale
							}
						end,
						text_extents = function( text )
							if type( text ) ~= "string" then
								error( "text expected", 2 )
							end
							pangocairo.pango_layout_set_text( layout, text, -1 )
							local rect = ffix.new( "PangoRectangle[1]" )
							pangocairo.pango_layout_get_pixel_extents( layout, nil, rect )
							return {
								width = rect[ 0 ].width * downscale * xscale * fonthack_scale,
								height = rect[ 0 ].height * downscale * yscale * fonthack_scale
							}
						end,
						
						text_to_shape = function( text )
							if type( text ) ~= "string" then
								error( "text expected", 2 )
							end
							pangocairo.cairo_save( context )
							pangocairo.cairo_scale( context, downscale * xscale * fonthack_scale, downscale * yscale * fonthack_scale )
							pangocairo.pango_layout_set_text( layout, text, -1 )
							pangocairo.pango_cairo_layout_path( context, layout )
							pangocairo.cairo_restore( context )
							local shape, shape_n = { }, 0
							local path = ffix.gc( pangocairo.cairo_copy_path( context ), pangocairo.cairo_path_destroy )
							if(path[ 0 ].status == ffix.C.CAIRO_STATUS_SUCCESS2) then
								local i, cur_type, last_type = 0
								while(i < path[ 0 ].num_data) do
									cur_type = path[ 0 ].data[ i ].header.type
									if cur_type == ffix.C.CAIRO_PATH_MOVE_TO then
										if cur_type ~= last_type then
											shape_n = shape_n + 1
											shape[ shape_n ] = "m"
										end
										shape[ shape_n + 1 ] = ke4.math.round( path[ 0 ].data[ i + 1 ].point.x, FP_PRECISION )
										shape[ shape_n + 2 ] = ke4.math.round( path[ 0 ].data[ i + 1 ].point.y, FP_PRECISION )
										shape_n = shape_n + 2
									elseif cur_type == ffix.C.CAIRO_PATH_LINE_TO then
										if cur_type ~= last_type then
											shape_n = shape_n + 1
											shape[ shape_n ] = "l"
										end
										shape[ shape_n + 1 ] = ke4.math.round( path[ 0 ].data[ i + 1 ].point.x, FP_PRECISION )
										shape[ shape_n + 2 ] = ke4.math.round( path[ 0 ].data[ i + 1 ].point.y, FP_PRECISION )
										shape_n = shape_n + 2
									elseif cur_type == ffix.C.CAIRO_PATH_CURVE_TO then
										if cur_type ~= last_type then
											shape_n = shape_n + 1
											shape[ shape_n ] = "b"
										end
										shape[ shape_n + 1] = ke4.math.round( path[ 0 ].data[ i + 1 ].point.x, FP_PRECISION )
										shape[ shape_n + 2] = ke4.math.round( path[ 0 ].data[ i + 1 ].point.y, FP_PRECISION )
										shape[ shape_n + 3] = ke4.math.round( path[ 0 ].data[ i + 2 ].point.x, FP_PRECISION )
										shape[ shape_n + 4] = ke4.math.round( path[ 0 ].data[ i + 2 ].point.y, FP_PRECISION )
										shape[ shape_n + 5] = ke4.math.round( path[ 0 ].data[ i + 3 ].point.x, FP_PRECISION )
										shape[ shape_n + 6] = ke4.math.round( path[ 0 ].data[ i + 3 ].point.y, FP_PRECISION )
										shape_n = shape_n + 6
									elseif cur_type == ffix.C.CAIRO_PATH_CLOSE_PATH then
										if cur_type ~= last_type then
											shape_n = shape_n + 1
											shape[ shape_n ] = "c"
										end
									end
									last_type = cur_type
									i = i + path[ 0 ].data[ i ].header.length
								end
							end
							pangocairo.cairo_new_path( context )
							return table.concat( shape, " " )
						end
					}
				end
			end,
			
			list_fonts = function( with_filenames )
				-- Lists available system fonts
				if with_filenames ~= nil and type( with_filenames ) ~= "boolean" then
					error( "optional boolean expected", 2 )
				end
				local fonts = { n = 0 }
				if ffix.os == "Windows" then
					local plogfont = ffix.new( "LOGFONTW[1]" )
					plogfont[ 0 ].lfCharSet = ffix.C.DEFAULT_CHARSET2
					plogfont[ 0 ].lfFaceName[ 0 ] = 0	-- Empty string
					plogfont[ 0 ].lfPitchAndFamily = ffix.C.DEFAULT_PITCH2 + ffix.C.FF_DONTCARE2
					local fontname, style, font
					ffix.C.EnumFontFamiliesExW( ffix.gc( ffix.C.CreateCompatibleDC( nil ), ffix.C.DeleteDC), plogfont, function( penumlogfont, _, fonttype, _)
						fontname, style = utf16_to_utf8( penumlogfont[ 0 ].elfLogFont.lfFaceName ), utf16_to_utf8( penumlogfont[ 0 ].elfStyle )
						for i = 1, fonts.n do
							font = fonts[ i ]
							if font.name == fontname and font.style == style then
								goto win_font_found
							end
						end
						fonts.n = fonts.n + 1
						fonts[ fonts.n ] = {
							name = fontname,
							longname = utf16_to_utf8( penumlogfont[ 0 ].elfFullName ),
							style = style,
							type = fonttype == ffix.C.FONTTYPE_RASTER2 and "Raster" or fonttype == ffix.C.FONTTYPE_DEVICE2 and "Device" or fonttype == ffix.C.FONTTYPE_TRUETYPE2 and "TrueType" or "Unknown",
						}
						::win_font_found::
						return 1
					end, 0, 0 )
					if with_filenames then
						local function file_to_font( fontname, fontfile )
							for i = 1, fonts.n do
								font = fonts[ i ]
								if fontname == font.name:gsub( "^@", "", 1 ) or fontname == format( "%s %s", font.name:gsub( "^@", "", 1 ), font.style ) or fontname == font.longname:gsub( "^@", "", 1 ) then
									font.file = fontfile
								end
							end
						end
						local pregkey, fontfile = ffix.new( "HKEY[1]" )
						if advapix.RegOpenKeyExA( ffix.cast( "HKEY", ffix.C.HKEY_LOCAL_MACHINE2 ), "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, ffix.C.KEY_READ2, pregkey ) == ffix.C.ERROR_SUCCESS2 then
							local regkey = ffix.gc( pregkey[ 0 ], advapix.RegCloseKey )
							local value_index, value_name, pvalue_name_size, value_data, pvalue_data_size = 0, ffix.new( "wchar_t[16383]" ), ffix.new( "DWORD[1]" ), ffix.new( "BYTE[65536]" ), ffix.new( "DWORD[1]" )
							while true do
								pvalue_name_size[ 0 ], pvalue_data_size[ 0 ] = ffix.sizeof( value_name ) / ffix.sizeof( "wchar_t" ), ffix.sizeof( value_data )
								if advapix.RegEnumValueW( regkey, value_index, value_name, pvalue_name_size, nil, nil, value_data, pvalue_data_size ) ~= ffix.C.ERROR_SUCCESS2 then
									break
								else
									value_index = value_index + 1
								end
								fontname = utf16_to_utf8( value_name ):gsub( "(.*) %(.-%)$", "%1", 1 )
								fontfile = utf16_to_utf8( ffix.cast("wchar_t*", value_data ) )
								file_to_font( fontname, fontfile )
								if fontname:find( " & " ) then
									for fontname in fontname:gmatch( "(.-) & " ) do
										file_to_font( fontname, fontfile )
									end
									file_to_font( fontname:match( ".* & (.-)$" ), fontfile )
								end
							end
						end
					end
				else	-- Unix
					if not fontconfig then
						error( "fontconfig library couldn't be loaded", 2 )
					end
					local fontset = ffix.gc( fontconfig.FcFontList( fontconfig.FcInitLoadConfigAndFonts( ),
																ffix.gc( fontconfig.FcPatternCreate( ), fontconfig.FcPatternDestroy ),
																ffix.gc( fontconfig.FcObjectSetBuild( "family", "fullname", "style", "outline", with_filenames and "file" or nil, nil ), fontconfig.FcObjectSetDestroy ) ),
											fontconfig.FcFontSetDestroy )
					local font, family, fullname, style, outline, file
					local cstr, cbool = ffix.new( "FcChar8*[1]" ), ffix.new( "FcBool[1]" )
					for i = 0, fontset[ 0 ].nfont - 1 do
						font = fontset[ 0 ].fonts[ i ]
						family, fullname, style, outline, file = nil
						if fontconfig.FcPatternGetString( font, "family", 0, cstr ) == ffix.C.FcResultMatch then
							family = ffix.string( cstr[ 0 ] )
						end
						if fontconfig.FcPatternGetString( font, "fullname", 0, cstr ) == ffix.C.FcResultMatch then
							fullname = ffix.string( cstr[ 0 ] )
						end
						if fontconfig.FcPatternGetString( font, "style", 0, cstr ) == ffix.C.FcResultMatch then
							style = ffix.string( cstr[ 0 ] )
						end
						if fontconfig.FcPatternGetBool( font, "outline", 0, cbool ) == ffix.C.FcResultMatch then
							outline = cbool[ 0 ]
						end
						if fontconfig.FcPatternGetString( font, "file", 0, cstr ) == ffix.C.FcResultMatch then
							file = ffix.string( cstr[ 0 ] )
						end
						if family and fullname and style and outline then
							fonts.n = fonts.n + 1
							fonts[ fonts.n ] = {
								name = family,
								longname = fullname,
								style = style,
								type = outline == 0 and "Raster" or "Outline",
								file = file
							}
						end
					end
				end
				table.sort( fonts, function( font1, font2 )
					if font1.name == font2.name then
						return font1.style < font2.style
					else
						return font1.name < font2.name
					end
				end )
				return fonts
			end
		}
	}
	
	return ke4
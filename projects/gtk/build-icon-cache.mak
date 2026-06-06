

gtkbuiltincache.h:
	@"$(OUTDIR)gtk-update-icon-cache --force --ignore-theme-index 	\
	   --source builtin_icons stock-icons
	move iconcache.h gtkbuiltincache.h
# Using a makefile for this to get gtk_public_h_sources from the makefile in the source 
# Can't just use the makefile in the source because it uses the old perl version of 
# glib-mkenums and the version of glib we have has the new python version of the script

!include $(TOP)gtk/makefile.msc.in

makefile.msc:

gtktypebuiltins.h_ : $(gtk_public_h_sources)
	echo #ifndef __GTK_TYPE_BUILTINS_H__ > gtktypebuiltins.h_
	echo #define __GTK_TYPE_BUILTINS_H__ >> gtktypebuiltins.h_
	echo #include "glib-object.h" >> gtktypebuiltins.h_
	echo G_BEGIN_DECLS >> gtktypebuiltins.h_
	python3 $(GLIB_TOOLS)glib-mkenums \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void);\n#define GTK_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n#endif /* __GTK_TYPE_BUILTINS_H__ */" \
		$(gtk_public_h_sources) >> gtktypebuiltins.h_


gtktypebuiltins.c_ : $(gtk_public_h_sources)
	python3 $(GLIB_TOOLS)glib-mkenums \
		--fhead "#define GTK_ENABLE_BROKEN\n#include \"gtk.h\"\n#include \"gtkprivate.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(gtk_public_h_sources) > gtktypebuiltins.c_


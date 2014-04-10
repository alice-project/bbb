#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifndef __MAP_H__
#define __MAP_H__

#define ANTS_MAP_TYPE                  (ants_map_get_type())
#define ANTS_MAP_GET_PRIVATE(obj)      (G_TYPE_INSTANCE_GET_PRIVATE((obj), ANTS_MAP_TYPE, AntsMapPrivate))
#define ANTS_MAP(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),  ANTS_MAP_TYPE, AntsMap))
#define ANTS_MAP_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),   ANTS_MAP, AntsMapClass))
#define ANTS_IS_ANTS_MAP(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),  ANTS_MAP_TYPE))
#define ANTS_IS_ANTS_MAP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),   ANTS_MAP_TYPE))
#define ANTS_MAP_GET_CLASS             (G_TYPE_INSTANCE_GET_CLASS((obj),   ANTS_MAP_TYPE, AntsMapClass))


struct _AntsMap
{
    GtkDrawingArea parent;

    /* private */
};

struct _AntsMapClass
{
    GtkDrawingAreaClass parent_class;
};

struct _AntsMapPrivate
{
     struct tm *time; /* the time on the clock face */
};

typedef struct _AntsMapPrivate AntsMapPrivate;

typedef struct _AntsMap            AntsMap;
typedef struct _AntsMapClass       AntsMapClass;

GtkWidget* ants_map_new(void);
static void ants_map_init(AntsMap *map);
static void ants_map_class_init(AntsMapClass *map_class);

#endif

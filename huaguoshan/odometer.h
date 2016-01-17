#ifndef __ODOMETER_H__
#define __ODOMETER_H__

#include <gtk/gtk.h>
#include <cairo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

G_BEGIN_DECLS

/* Standart GObject macros */
#define TYPE_ODOMETER (odometer_get_type())
#define ODOMETER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_ODOMETER, Odometer))

#define ODOMETER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_ODOMETER, OdometerClass))
#define IS_ODOMETER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_ODOMETER))

#define IS_ODOMETER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_ODOMETER))
#define ODOMETER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_ODOMETER, OdometerClass))

typedef struct _Odometer Odometer;
typedef struct _OdometerClass OdometerClass;
typedef struct _OdometerPrivate OdometerPrivate;

struct _Odometer {
    GtkWidget widget;

    /*< Private >*/
    OdometerPrivate *priv;
};

struct _OdometerClass {
    GtkWidgetClass parent_class;
    
    void (* odometer) (Odometer *ooo);
};


/* Public API */
GType odometer_get_type(void) G_GNUC_CONST;
GtkWidget *odometer_new(void);
void odometer_set_speed(Odometer *odometer, gdouble speed);
gdouble odometer_get_speed(Odometer *odometer);

void odometer_set_mileage(Odometer *odometer, gdouble mileage);
gdouble odometer_get_mileage(Odometer *odometer);

G_END_DECLS

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ODOMETER_H__ */


#ifndef DEF_DATE_MODULE_HPP
#define DEF_DATE_MODULE_HPP

#include <gtkmm.h>

class DateModule : public Gtk::Box {
public:
  DateModule();
  ~DateModule();

protected:
  Gtk::Label m_time_label;
  Gtk::Label m_date_label;

  bool update_labels();
};

#endif // !DEF_DATE_MODULE_HPP

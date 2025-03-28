#pragma once

class Captureable {
public:
  virtual bool get_captured() = 0;
  virtual void set_captured(bool captured) = 0;
};
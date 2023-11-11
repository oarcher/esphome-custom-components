#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/ota/ota_backend.h"

#include <memory>
#include <utility>
#include <string>

namespace esphome {
namespace ota_http {

static const char *const TAG = "ota_http";

class OtaHttpComponent : public Component {
 public:
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
  void set_url(std::string url) {
    this->url_ = std::move(url);
    this->secure_ = this->url_.rfind("https:", 0) == 0;
  }
  void set_timeout(uint64_t timeout) { this->timeout_ = timeout; }
  void flash();
  virtual int http_init() { return -1; };
  virtual size_t http_read(uint8_t *buf, size_t len) { return 0; };
  virtual void http_end(){};

 protected:
  std::string url_;
  bool secure_;
  size_t body_length_ = 0;
  size_t bytes_read_ = 0;
  uint64_t timeout_{1000 * 60 * 10};            // must match CONF_TIMEOUT in __init__.py
  const uint16_t http_recv_buffer_ = 1000;      // the firwmware GET chunk size
  const uint16_t max_http_recv_buffer_ = 1024;  // internal max http buffer size must be > HTTP_RECV_BUFFER_ (TLS
                                                // overhead) and must be a power of two from 512 to 4096
  bool update_started_ = false;
  static const std::unique_ptr<ota::OTABackend> BACKEND;
  void cleanup_();
};

template<typename... Ts> class OtaHttpFlashAction : public Action<Ts...> {
 public:
  OtaHttpFlashAction(OtaHttpComponent *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, url)
  TEMPLATABLE_VALUE(uint16_t, timeout)

  void play(Ts... x) override {
    this->parent_->set_url(this->url_.value(x...));
    if (this->timeout_.has_value()) {
      this->parent_->set_timeout(this->timeout_.value(x...));
    }
    this->parent_->flash();
  }

 protected:
  OtaHttpComponent *parent_;
};

}  // namespace ota_http
}  // namespace esphome

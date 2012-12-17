
#include <linux/types.h>

uint32_t
sec_hal_drm_enter(uint32_t thr_id,
                  uint32_t *session_id);

uint32_t
sec_hal_drm_exit(uint32_t id_cnt,
                 uint32_t session_ids[]);

uint32_t
sec_hal_drm_set_entit_key(uint32_t session_id,
                          uint32_t key_len,
                          uint8_t *key);

uint32_t
sec_hal_drm_derive_cw(uint32_t session_id,
                      uint32_t ecm_len,
                      uint8_t *ecm,
                      uint32_t *flags);

uint32_t
sec_hal_drm_decrypt_video(uint32_t session_id,
                          uint8_t *iv_in,
                          uint32_t iv_len,
                          uint32_t input_len,
                          uint32_t input_phys_addr,
                          uint32_t output_phys_addr,
                          uint32_t output_offset,
                          uint32_t *output_len,
                          uint8_t *iv_out);

uint32_t
sec_hal_drm_decrypt_audio(uint32_t session_id,
                          uint8_t *iv_in,
                          uint32_t iv_len,
                          uint32_t input_len,
                          uint32_t input_phys_addr,
                          uint32_t output_phys_addr,
                          uint32_t *output_len,
                          uint8_t *iv_out);

uint32_t
sec_hal_drm_wrap_keybox(uint8_t *keybox,
                        uint32_t keybox_size,
                        uint8_t *wrapped_keybox,
                        uint32_t *wrapped_keybox_size,
                        uint8_t *transport_key,
                        uint32_t transport_key_size);

uint32_t
sec_hal_drm_install_keybox(uint8_t *keybox,
                           uint32_t keybox_size);

uint32_t
sec_hal_drm_is_keybox_valid(void);

uint32_t
sec_hal_drm_get_random(uint32_t size,
                       uint8_t *random);

uint32_t
sec_hal_drm_get_device_id(uint32_t id_len,
                          uint8_t *id,
                          uint32_t *out_size);

uint32_t
sec_hal_drm_get_key_data(uint32_t key_data_len,
                         uint8_t *key_data,
                         uint32_t *out_size);


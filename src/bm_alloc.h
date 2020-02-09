#ifndef _BM_ALLOC_H_
#define _BM_ALLOC_H_

#ifdef ESP32
#ifdef BOARD_HAS_PSRAM
#define ESP32_WITH_PSRAM
#endif
#endif

#ifdef ESP32_WITH_PSRAM
#define bm_check_integrity(print_errors) heap_caps_check_integrity(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, print_errors)
#define bm_malloc(size) heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define bm_free(p) heap_caps_free(p)
#else
inline bool bm_check_integrity(bool print_errors) { return true; }
#define bm_malloc(size) malloc(size)
#define bm_free(p) free(p)
#endif

#ifdef ESP32_WITH_PSRAM
inline size_t bm_max_alloc() { return ESP.getMaxAllocPsram(); }
#elif defined(ESP32)
inline size_t bm_max_alloc() { return ESP.getMaxAllocHeap(); }
#elif defined(ESP8266)
inline size_t bm_max_alloc() { return ESP.getMaxFreeBlockSize(); }
#else
inline size_t bm_max_alloc() { return 0; }
#endif

#endif

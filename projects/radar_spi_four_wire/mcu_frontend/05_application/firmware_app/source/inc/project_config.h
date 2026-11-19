/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include <stdint.h>

/* Pure-front-end firmware identity. */
#define PROJECT_VERSION_TAG                      "f1_1_sync"
#define PROJECT_VERSION_MAJOR                    (1u)
#define PROJECT_VERSION_MINOR                    (1u)
#define PROJECT_VERSION_PATCH                    (0u)
#define PROJECT_BOARD_VARIANT                    (4u)

/* P0=SPI CS, P1=SYNC input, P12=READY output. UART0 is disabled. */
#define PROJECT_PROTOCOL_HEADER0                 (0xA0u)
#define PROJECT_PROTOCOL_HEADER1                 (0x55u)
#define PROJECT_TRACKING_RUN_INTERVAL_DEFAULT_MS (50u)
#define PROJECT_TRACKING_RUN_INTERVAL_MIN_MS     (1u)
#define PROJECT_TRACKING_RUN_INTERVAL_MAX_MS     (60000u)
#define PROJECT_TIMEBASE_TICK_US                 (1000u)

/* Separate Keil selftest target generates payload in RAM, never enabling RF. */
#ifndef RADAR_SYNC_SELFTEST
#define RADAR_SYNC_SELFTEST (0)
#endif

/* Front-end radar acquisition. */
#define PROJECT_FMCW_BANDWIDTH_DEFAULT_MHZ       (1400u)
#define PROJECT_FMCW_BANDWIDTH_MIN_MHZ           (80u)
#define PROJECT_FMCW_BANDWIDTH_MAX_MHZ           (4000u)
#define PROJECT_ADC_SAMPLE_LENGTH                (512u)
#define PROJECT_ADC_CAPTURE_CHIRP_COUNT          (24u)

/* Start fixed at 0 degrees; set 1 only after the fixed-beam link is verified. */
#define PROJECT_SYNC_SCAN_ENABLE                  (0u)

/* Fixed 19-beam loop: -45 deg .. +45 deg in 5 deg steps. */
#define PROJECT_FRONTEND_BEAM_COUNT               (19u)
#define PROJECT_FRONTEND_INITIAL_BEAM_INDEX       (9u)  /* 0 deg */

/* Ping-pong ADC/SPI stream RAM. One frame = samples * chirps * (I16+Q16). */
#define PROJECT_ADC_BYTES_PER_COMPLEX_SAMPLE      (4UL)
#define PROJECT_ADC_BYTES_PER_CHIRP               \
    ((PROJECT_ADC_SAMPLE_LENGTH) *                \
     PROJECT_ADC_BYTES_PER_COMPLEX_SAMPLE)
#define PROJECT_ADC_FRAME_BYTES                   \
    ((PROJECT_ADC_SAMPLE_LENGTH) *                \
     (PROJECT_ADC_CAPTURE_CHIRP_COUNT) *          \
     PROJECT_ADC_BYTES_PER_COMPLEX_SAMPLE)
#define PROJECT_SPI_FRAME_METADATA_WORDS          (16UL)
#define PROJECT_SPI_FRAME_METADATA_BYTES          \
    (PROJECT_SPI_FRAME_METADATA_WORDS * 2UL)
#define PROJECT_SPI_FRAME_METADATA_OFFSET         \
    (PROJECT_ADC_BYTES_PER_CHIRP -                \
     PROJECT_SPI_FRAME_METADATA_BYTES)
#define PROJECT_SPI_PAYLOAD_CHIRP_COUNT           \
    (PROJECT_ADC_CAPTURE_CHIRP_COUNT - 1u)
#define PROJECT_SPI_PAYLOAD_BYTES                 \
    (PROJECT_SPI_PAYLOAD_CHIRP_COUNT *            \
     PROJECT_ADC_BYTES_PER_CHIRP)
#define PROJECT_SPI_STREAM_BYTES                  \
    (PROJECT_SPI_FRAME_METADATA_BYTES +           \
     PROJECT_SPI_PAYLOAD_BYTES)
#define PROJECT_SPI_FRAME_MAGIC                   (0xA610u)
#define PROJECT_SPI_FRAME_METADATA_VERSION        (3u)
#define PROJECT_RANGE_RESOLUTION_NUMERATOR_UM_MHZ (150000000UL)
#define PROJECT_ADC_BUFFER_A_ADDR                 (0x20008000UL)
#define PROJECT_ADC_BUFFER_B_ADDR                 \
    (PROJECT_ADC_BUFFER_A_ADDR + PROJECT_ADC_FRAME_BYTES)
#define PROJECT_ADC_BUFFER_END_ADDR               \
    (PROJECT_ADC_BUFFER_B_ADDR + PROJECT_ADC_FRAME_BYTES)
#define PROJECT_FRONTEND_RAM_END_ADDR              (0x20020000UL)

/* SPI0 raw ADC output: 16-bit words, configured for measured ~12.5 MHz SCLK. */
#define PROJECT_SPI_STREAM_DMA_CHANNEL            (3u)
#define PROJECT_SPI_STREAM_WORD_BITS              (16u)
#define PROJECT_SPI_STREAM_CLOCK_HZ               (12500000UL)

#if (PROJECT_SPI_STREAM_WORD_BITS != 16u) || (PROJECT_SPI_STREAM_CLOCK_HZ != 12500000UL)
#error "f1_0 SPI stream is fixed at 16-bit / 12.5 MHz"
#endif
#if (PROJECT_FRONTEND_BEAM_COUNT != 19u)
#error "f1_0 requires exactly 19 fixed beams"
#endif
#if (PROJECT_ADC_SAMPLE_LENGTH == 0u) || (PROJECT_ADC_CAPTURE_CHIRP_COUNT == 0u)
#error "ADC sample length and chirp count must be nonzero"
#elif (PROJECT_ADC_CAPTURE_CHIRP_COUNT < 2u)
#error "SPI metadata uses the first chirp, so at least two chirps are required"
#elif ((PROJECT_ADC_SAMPLE_LENGTH) > \
     ((PROJECT_FRONTEND_RAM_END_ADDR - PROJECT_ADC_BUFFER_A_ADDR) / \
      (2UL * PROJECT_ADC_BYTES_PER_COMPLEX_SAMPLE) / \
      (PROJECT_ADC_CAPTURE_CHIRP_COUNT)))
#error "ADC ping-pong buffers exceed the configured on-chip RAM range"
#endif
#if (PROJECT_FMCW_BANDWIDTH_MIN_MHZ > PROJECT_FMCW_BANDWIDTH_DEFAULT_MHZ) || \
    (PROJECT_FMCW_BANDWIDTH_DEFAULT_MHZ > PROJECT_FMCW_BANDWIDTH_MAX_MHZ)
#error "default FMCW bandwidth must be inside the supported range"
#endif
#if (PROJECT_TRACKING_RUN_INTERVAL_MIN_MS == 0u) || \
    (PROJECT_TRACKING_RUN_INTERVAL_MIN_MS > PROJECT_TRACKING_RUN_INTERVAL_DEFAULT_MS) || \
    (PROJECT_TRACKING_RUN_INTERVAL_DEFAULT_MS > PROJECT_TRACKING_RUN_INTERVAL_MAX_MS)
#error "default tracking interval must be inside the supported range"
#endif

#endif /* PROJECT_CONFIG_H */

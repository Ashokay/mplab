#ifndef CANbus_Driver_z_H    /* Guard against multiple inclusion */
#define CANbus_Driver_z_H

#ifdef __cplusplus
extern "C" {
#endif
    
    void CANbus_init_1(void);
    bool CANbus_write_1(uint32_t Sadr, uint8_t Sdata_L, uint8_t * Sdata);
    bool CANbus_read_1(uint32_t *msg_id, uint8_t *length, uint8_t *Rdata);

#ifdef __cplusplus
}
#endif

#endif /* driver_CANbus_VUC_H */
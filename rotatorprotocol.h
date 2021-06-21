#ifndef ROTATORPROTOCOL_H
#define ROTATORPROTOCOL_H
#include <QByteArray>

enum AXIS {AZIMUTH=1, PITCH=2, BOTH=3};

class RotatorProtocol
{
private:
    struct control_frame
    {
        const unsigned char frame_header[2] = {0x55, 0xaa};
        const char addr = 0xfe;
        unsigned char len = 0;
        unsigned char func_code;
        unsigned char data[10];
        unsigned char sum_check;
    } cmd_frame;
    enum CMD_CODE {
        Speed=0x01,
        Position=0x10,
        AzimuthPoint=0x17,
        PitchPoint=0x18,
        AzimuthContinue=0x1e,
        PitchContinue=0x1f,
        Standby=0x40};
//private:
//    control_frame cmd_frame;
public:
    RotatorProtocol();
    int set_speed();
    int set_target_angle(int Azimuth, int Pitch, AXIS axis);
    void check_sum();
    QByteArray get_bitstring();
};

#endif // ROTATORPROTOCOL_H

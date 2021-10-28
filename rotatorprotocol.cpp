#include <QDebug>
#include "rotatorprotocol.h"

RotatorProtocol::RotatorProtocol()
{

}

int RotatorProtocol::set_target_angle(int Azimuth, int Pitch, Axis axis)
{
    this->cmd_frame.len = 0x6;
    this->cmd_frame.func_code = Position;
    unsigned char *p = (unsigned char *)&Azimuth;
    this->cmd_frame.data[0] = p[0];
    this->cmd_frame.data[1] = p[1];
    p = (unsigned char *)&Pitch;
    this->cmd_frame.data[2] = p[0];
    this->cmd_frame.data[3] = p[1];

    this->cmd_frame.data[4] = axis;

    this->check_sum();
    return 0;
}

void RotatorProtocol::check_sum()
{
    char sum = (int)this->cmd_frame.addr;
    sum = sum  + (int)cmd_frame.len;
    sum = sum + (int)cmd_frame.func_code;
    for (int i=0; i<cmd_frame.len-1; i++){
        sum += cmd_frame.data[i];
    }
    cmd_frame.sum_check = sum;
}

QByteArray RotatorProtocol::get_bitstring()
{
    QByteArray qa(QByteArray::fromRawData((char*)cmd_frame.frame_header, 2));

    qa.append((char)this->cmd_frame.addr);
    qa.append((char)this->cmd_frame.len);
    qa.append((char)this->cmd_frame.func_code);
    qa.append((char *)this->cmd_frame.data, cmd_frame.len - 1);
    qa.append((char)this->cmd_frame.sum_check);
    return qa;
}

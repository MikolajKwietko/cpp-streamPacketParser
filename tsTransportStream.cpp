#include "tsTransportStream.h"

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset()
{
}
int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{
  uint32_t tmp = *((uint32_t*)Input);
    tmp = xSwapBytes32(tmp);

    //SB
    uint32_t mask = 0b11111111000000000000000000000000;
    SB = tmp & mask;
    SB = SB >> 24;
    //E
    mask = 0b00000000100000000000000000000000;
    E = tmp & mask;
    E = E >> 23;
    //S
    mask = 0b00000000010000000000000000000000;
    S = tmp & mask;
    S = S >> 22;
    //T
    mask = 0b00000000001000000000000000000000;
    T = tmp & mask;
    T = T >> 21;
    //PID
    mask = 0b00000000000111111111111100000000;
    PID = tmp & mask;
    PID = PID >> 8;
    //TSC
    mask = 0b00000000000000000000000011000000;
    TSC = tmp & mask;
    TSC = TSC >> 6;
    //AFC
    mask = 0b00000000000000000000000000110000;
    AFC = tmp & mask;
    AFC = AFC >> 4;
    //CC
    mask = 0b00000000000000000000000000001111;
    CC = tmp & mask;

    return 1;
}
void xTS_PacketHeader::Print() const
{
  printf("TS: SB=%d E=%d S=%d T=%d PID=%d TSC=%d AFC=%d CC=%d ", SB, E, S, T, PID, TSC, AFC, CC);
}

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================

void xTS_AdaptationField::Reset()
{

}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AFC)
{
    AFL = Input[4];
    uint32_t TmpAFC = *((uint32_t*)(Input+4));
    uint8_t Flags = Input[5];

    //DC
    uint8_t mask = 0b10000000;
    DC = (mask & Flags) >> 7;
    //RA
    mask = 0b01000000;
    RA = (mask & Flags) >> 6;
    //SPI
    mask = 0b00100000;
    SPI = (mask & Flags) >> 5;
    //PR
    mask = 0b00010000;
    PR = (mask & Flags) >> 4;
    //OR
    mask = 0b00001000;
    OR = (mask & Flags) >> 3;
    //SPF
    mask = 0b00000100;
    SPF = (mask & Flags) >> 2;
    //TP
    mask = 0b00000010;
    TP = (mask & Flags) >> 1;
    //EX
    mask = mask = 0b00000001;
    EX = (mask & Flags); 
    
    return 1;
}

void xTS_AdaptationField::Print() const{
    printf("AF: AFL=%d DC=%d RA=%d SP=%d PR=%d OR=%d SPF=%d TP=%d EX=%d ", AFL, DC, RA, SPI, PR, OR, SPF, TP, EX);
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================

int32_t xPES_PacketHeader::Parse(const uint8_t* Input, int dataOffset){
  int currentByte = 188 - dataOffset;

  m_PacketStartCodePrefix = (Input[currentByte]<<16) + (Input[currentByte+1]<<8) + Input[currentByte+2];
  m_StreamId = Input[currentByte+3];
  m_PacketLength = (Input[currentByte+4]<<8) + Input[currentByte+5];
  m_HeaderLength = Input[currentByte+8];

  return 1;
}

void xPES_PacketHeader::Print() const{
  printf("PES: PSCP=%d SID=%d L=%d ", m_PacketStartCodePrefix, m_StreamId, m_PacketLength);
}

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t *TransportStreamPacket, xTS_PacketHeader TS_PacketHeader, int dataOffset, FILE* file){
  int currentByte = 188-dataOffset;
    for(int i=0; i<dataOffset; i++) {
        m_Buffer[m_BufferSize + i] = TransportStreamPacket[currentByte + i];
        
        uint8_t currByte = TransportStreamPacket[currentByte + i];
        fwrite(&currByte, 1, 1, file);

        m_BufferSize += 1;
    }
    if(TS_PacketHeader.hasAdaptationField() && TS_PacketHeader.getS() == 1){
      // m_LastContinuityCounter = TS_PacketHeader.getCC();
      return eResult::AssemblingStarted;
    }
    // if(!(TS_PacketHeader.getCC()==m_LastContinuityCounter+1)){
    //   return eResult::StreamPackedLost;
    // }
    m_LastContinuityCounter++;
    if(m_LastContinuityCounter == 16) {m_LastContinuityCounter = 0;}
    if(TS_PacketHeader.hasAdaptationField() && TS_PacketHeader.getS() == 0){
      printf("Len=%d ", getBufferSize()+14);
      BufferReset();
      return eResult::AssemblingFinished;
    }

    return eResult::AssemblingContinue;
}
#include "tsCommon.h"
#include "tsTransportStream.h"

int main(int argc, char *argv[ ], char *envp[ ])
{
  // open file
  FILE * file;
  file = fopen("../../example_new.ts", "rb");
  if(file == nullptr){
    printf("Error");
    return 0;
  }
  xTS_PacketHeader    TS_PacketHeader;
  xTS_AdaptationField TS_AdaptationField;
  xPES_PacketHeader   PES_PacketHeader;
  xPES_Assembler      PES_Assembler; 

  int dataOffset;
  int32_t TS_PacketId = 0;
  // create audio file
  FILE * clearaudio = fopen("../../audio.mp2", "wb");
  while(!feof(file))
  {
    // read from file
    
    uint8_t * TS_PacketBuffer;
    TS_PacketBuffer = (uint8_t *) malloc (sizeof(uint8_t)*188);
    size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, file);
    if(NumRead != xTS::TS_PacketLength) { break; }
    // TS_PacketHeader.Reset();
    
    // TS
    TS_PacketHeader.Parse(TS_PacketBuffer);
    if(TS_PacketHeader.getPID() == 136){
      printf("%010d ", TS_PacketId);
      TS_PacketHeader.Print();
      dataOffset = 184;

      //AdaptationField
      if (TS_PacketHeader.hasAdaptationField()) {
        TS_AdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAFC());
        TS_AdaptationField.Print();

        dataOffset = 184 - 1 - TS_AdaptationField.getAFLength();
      }
      //PES
      int PESLength;
      if(TS_PacketHeader.getPID() == 136){
        PESLength = dataOffset;
        if(TS_PacketHeader.getS() == 1){
          PES_PacketHeader.Parse(TS_PacketBuffer, dataOffset);
          PES_PacketHeader.Print();

          PESLength = dataOffset - 9 - PES_PacketHeader.getHeaderLength();
        }
        xPES_Assembler::eResult Result = PES_Assembler.AbsorbPacket(TS_PacketBuffer, TS_PacketHeader, PESLength, clearaudio);
        switch(Result)
        {
        case xPES_Assembler::eResult::StreamPackedLost : printf("PcktLost "); break;
        case xPES_Assembler::eResult::AssemblingStarted : printf("Started "); break;
        case xPES_Assembler::eResult::AssemblingContinue: printf("Continue "); break;
        case xPES_Assembler::eResult::AssemblingFinished: printf("Finished "); break;
        default: break;
        }
      }

      printf("\n");
    }
    TS_PacketId++;
  }
  fclose(clearaudio);
  fclose(file);
  return 0;
}
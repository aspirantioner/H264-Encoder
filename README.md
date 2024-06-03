H264 FILE Struct Parse
VCL(视频编码层)数据: sodb(String Of Data Bits)数据比特串
RBSP(Raw ByteSequence Payload)原始字节序列载荷 = sodb+byte_align();
EBSP(EncapsulationByte Sequence Packets)扩展字节序列载荷 = ADD0x03 for 0x00000000/01/02/03(RBSP) when H264 format is Annexb,other format is start bytes is naul length 
NAL(网络提取层)  = NALU-Header + EBSP
H264帧由NALU头和NALU主体组成。
NALU头由一个字节组成,它的语法如下:
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+

F: 1个比特.
  forbidden_zero_bit. 在 H.264 规范中规定了这一位必须为 0.

NRI: 2个比特.
  nal_ref_idc. 取00~11,似乎指示这个NALU的重要性,如00的NALU解码器可以丢弃它而不影响图像的回放,0～3，取值越大，表示当前NAL越重要，需要优先受到保护。如果当前NAL是属于参考帧的片，或是序列参数集，或是图像参数集这些重要的单位时，本句法元素必需大于0。

Type: 5个比特.
  nal_unit_type. 这个NALU单元的类型,1～12由H.264使用，24～31由H.264以外的应用使用,简述如下:

  0     没有定义
  1-23  NAL单元  单个 NAL 单元包
  1     不分区，非IDR图像的片
  2     片分区A
  3     片分区B
  4     片分区C (设计Slice的目的主要在于防止误码的扩散。因为不同的slice之间，其解码操作是独立的。某一   slice的解码过程所参考的数据（例如预测编码）不能越过slice的边界)。
  5     IDR图像中的片
  6     补充增强信息单元（SEI）
  7     SPS
  8     PPS
  9     序列结束
  10    序列结束
  11    码流借宿
  12    填充
  13-23 保留

  24    STAP-A   单一时间的组合包
  25    STAP-B   单一时间的组合包
  26    MTAP16   多个时间的组合包
  27    MTAP24   多个时间的组合包
  28    FU-A     分片的单元
  29    FU-B     分片的单元
  30-31 没有定义
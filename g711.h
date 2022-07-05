
int pcmu_codec_encoder(const void * _from, unsigned * fromLen, void * _to,   unsigned int * toLen);
int pcmu_codec_decoder(const void * _from, unsigned * fromLen, void * _to,   unsigned int * toLen);
int pcmu_decode(void * _to,  void *_from, unsigned int fromLen);
int pcma_decode(void * _to,  void *_from, unsigned int fromLen);


#define Outp32(addr, data) (*(volatile u32 *)(addr) = (data))
#define Outp16(addr, data) (*(volatile u16 *)(addr) = (data))
#define Outp8(addr, data)  (*(volatile u8 *)(addr) = (data))
#define Inp32(addr, data) (data = (*(volatile u32 *)(addr)))
#define Inp16(addr, data) (data = (*(volatile u16 *)(addr)))
#define Inp8(addr, data)  (data = (*(volatile u8 *)(addr)))

#define memOutp32(offset, x) Outp32(offset, x)
#define memOutp8(offset, x) Outp8(offset, x)

//#define dmaOutp32(offset, x) Outp32(m_uBaseAddr+offset, x)
//#define dmaInp32(offset, x) Inp32(m_uBaseAddr+offset, x)

//#define dmaChOutp32(offset, x) Outp32(m_uBaseAddr+m_uChAddr+offset, x)
//#define dmaChInp32(offset, x) Inp32(m_uBaseAddr+m_uChAddr+offset, x)

#define WRITEWORD(addr, d) ((*(volatile u8*)(addr)=(u8)(d&0xff)),\
							(*(volatile u8*)(addr+1)=(u8)((d>>8)&0xff)), \
							(*(volatile u8*)(addr+2)=(u8)((d>>16)&0xff)), \
							(*(volatile u8*)(addr+3)=(u8)((d>>24)&0xff)))


static u32 m_uCodeOffset;
static u32 m_uCodeStAddr;



/* DMAMOV CCR, ...  */
static void encodeDmaMoveChCtrl(void)
{
	u8 uDir = 1; // Channel Control
	u32 x = *(u32*)&m_oCtrReg;
	u8 uInsBytes[6];

	DBG(("CCR x = 0x%x\n", x));
	
	uInsBytes[0] = (u8)(0xbc);
	uInsBytes[1] = (u8)(0x0|uDir);
	uInsBytes[2] = (u8)((x>>0)&0xff);
	uInsBytes[3] = (u8)((x>>8)&0xff);
	uInsBytes[4] = (u8)((x>>16)&0xff);
	uInsBytes[5] = (u8)((x>>24)&0xff);	

	for(u32 i=0; i<6; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	m_uCodeOffset += 6;

//	dmaChOutp32(DMA_CH_CONTROL, x);
}

/* DMAMOV SAR, uStAddr
 * DMAMOV DAR, uStAddr   */
static void encodeDmaMove(bool bSrcDir, u32 uStAddr)
{
	u8 uDir = (bSrcDir) ? 0 : 2; // src addr=0, dst addr=2
	u8 uInsBytes[6];

	uInsBytes[0] = (u8)(0xbc);
	uInsBytes[1] = (u8)(0x0|uDir);
	uInsBytes[2] = (u8)((uStAddr>>0)&0xff);
	uInsBytes[3] = (u8)((uStAddr>>8)&0xff);
	uInsBytes[4] = (u8)((uStAddr>>16)&0xff);
	uInsBytes[5] = (u8)((uStAddr>>24)&0xff);

	for(u32 i=0; i<6; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	m_uCodeOffset += 6;
#if 0
	if (bSrcDir)
	{
		dmaChOutp32(DMA_CH_SRCADDR, uStAddr);
	}
	else
	{
		dmaChOutp32(DMA_CH_DSTADDR, uStAddr);
	}
#endif
}

/* DMALD, DMALDS, DMALDB  */
static void encodeDmaLoad(void)
{
	u8 bs;
	u8 x;
	u8 uInsBytes[1];

	if (m_eDir == DMA_M2M)
	{
		bs=0; x=0;
	}
	else if ((m_eDir == DMA_M2P) && (m_uBurstSz == 1)) // single transfer only
	{
		bs=0; x=1;
	}
	else if ((m_eDir == DMA_M2P) && (m_uBurstSz != 1)) // burst transfer only
	{
		bs=1; x=1;
	}
	else
	{
		assert(0);
	}

	uInsBytes[0] = (u8)(0x04|(bs<<1)|(x<<0));

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;	
}

/* DMALDPS, DMALDPB (Load Peripheral)  */
static void encodeDmaLoadPeri(void)
{
	assert(m_eDir == DMA_P2M);
	
	u8 bs;
	u8 uInsBytes[2];

	bs = (m_uBurstSz == 1) ? 0 : 1; // single -> 0, burst -> 1
	
	uInsBytes[0] = (u8)(0x25|(bs<<1));
	uInsBytes[1] = (u8)(0x00|((m_uDreqSrc&0x1f)<<3)); 

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	
	m_uCodeOffset += 2;
}

/* DMAST, DMASTS, DMASTB  */
static void encodeDmaStore(void)
{
	u8 bs;
	u8 x;
	u8 uInsBytes[1];

	if (m_eDir == DMA_M2M)
	{
		bs=0; x=0;
	}
	else if ((m_eDir == DMA_P2M) && (m_uBurstSz == 1)) // single transfer only
	{
		bs=0; x=1;
	}
	else if ((m_eDir == DMA_P2M) && (m_uBurstSz != 1)) // burst transfer only
	{
		bs=1; x=1;
	}
	else
	{
		assert(0);
	}
	
	uInsBytes[0] = (u8)(0x08|(bs<<1)|(x<<0));

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;	
}

/* DMASTPS, DMASTPB (Store and notify Peripheral)  */
static void encodeDmaStorePeri(void)
{
	assert(m_eDir == DMA_M2P);
	
	u8 bs;
	u8 uInsBytes[2];

	bs = (m_uBurstSz == 1) ? 0 : 1; // single -> 0, burst -> 1
	
	uInsBytes[0] = (u8)(0x29|(bs<<1));
	uInsBytes[1] = (u8)(0x00|((m_uDreqSrc&0x1f)<<3));

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	
	m_uCodeOffset += 2;
}

/* DMASTZ  */
static void encodeDmaStoreZero(void)
{
	u8 uInsBytes[1];
	
	uInsBytes[0] = (u8)(0x0c);

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;
}

/* DMALP  */
static void encodeDmaLoop(u8 uLoopCnt, u8 uIteration)
{
	assert(uLoopCnt < 2); // Lc0 or Lc1
	u8 uInsBytes[2];

	u8 uIter = uIteration-1;

	m_bFiniteLoop = true; // used for DMALPEND
	uInsBytes[0] = (u8)(0x20|(uLoopCnt<<1));
	uInsBytes[1] = (u8)(uIter);	

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	
	m_uCodeOffset += 2;
	m_uLoopStOffset[uLoopCnt] = m_uCodeOffset;
#if 0
	if (uLoopCnt == 0)
	{
		dmaChOutp32(DMA_CH_LoopCount0, uIter);
	}
	else
	{
		dmaChOutp32(DMA_CH_LoopCount1, uIter);
	}
#endif
}

/* DMALPFE  */
static void encodeDmaLoopForever(u8 uLoopCnt)
{
	assert(uLoopCnt < 2); // Lc0 or Lc1
	m_bFiniteLoop = false; // used for DMALPEND
	m_uLoopStOffset[uLoopCnt] = m_uCodeOffset;
}

/* DMALPEND, DMALPENDS, DMALPENDB  */
static void encodeDmaLoopEnd(u8 uLoopCnt)
{
	assert(uLoopCnt < 2); // Lc0 or Lc1
	
	u8 bs;
	u8 x;
	u8 uInsBytes[2];
	u8 uBwJump;

	u8 nf = (m_bFiniteLoop == true) ? 1 : 0;

	uBwJump = (u8)(m_uCodeOffset - m_uLoopStOffset[uLoopCnt]);
	m_uLoopStOffset[uLoopCnt] = 0;

	if (m_eDir == DMA_M2M)
	{
		bs=0; x=0;
	}
	else if (m_uBurstSz == 1) // single transfer only
	{
		/// temp bs=0; x=1;
		bs=0; x=0;
	}
	else if (m_uBurstSz != 1) // burst transfer only
	{
		bs=1; x=1;
	}
	else
	{
		assert(0);
	}
	
	uInsBytes[0] = (u8)(0x28|(nf<<4)|(uLoopCnt<<2)|(bs<<1)|x);
	uInsBytes[1] = (u8)(uBwJump);

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	
	m_uCodeOffset += 2;	
}

/*  DMAWFP, DMAWFPS, DMAWFPB (Wait For Peripheral) */
static void encodeDmaWaitForPeri(void)
{	
	assert(m_eDir != DMA_M2M);
	u8 bs;
	u8 p;
	u8 uInsBytes[2];

	if (m_uBurstSz == 1) // single transfer only
	{
		bs=0; p=0;
	}
	else if (m_uBurstSz != 1) // burst transfer only
	{
		bs=1; p=0;
	}
	else // both single transfer and burst one
	{
		bs=0; p=1;
	}
	
	uInsBytes[0] = (u8)(0x30|(bs<<1)|p);
	uInsBytes[1] = (u8)(0x00|((m_uDreqSrc&0x1f)<<3));

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 2;
}

/* DMAWFE (Wait For Event) */
static void encodeDmaWaitForEvent(u8 uEventNum) // 0~31
{
	assert(uEventNum < 32);
	
	u8 uInsBytes[2];
	
	uInsBytes[0] = (u8)(0x36);
	uInsBytes[1] = (u8)((uEventNum<<3)|0x2); // for cache coherency, invalid is issued.

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 2;
}

/*  DMAFLUSHP (Flush and notify Peripheral) */
static void encodeDmaFlushPeri(void)
{
	assert(m_eDir != DMA_M2M);
	u8 uInsBytes[2];

	uInsBytes[0] = (u8)(0x35); 
	uInsBytes[1] = (u8)(0x00|((m_uDreqSrc&0x1f)<<3));

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	
	m_uCodeOffset += 2;
}

/* DMAEND */
static void encodeDmaEnd(void)
{
	memOutp8(m_uCodeStAddr+m_uCodeOffset, 0x00);
	m_uCodeOffset += 1;	
}

/* DMAADDH (Add Halfword) */
static void encodeDmaAddHalfword(bool bSrcDir, u16 uStAddr)
{
	u8 uDir = (bSrcDir) ? 0 : 1; // src addr=0, dst addr=1
	u8 uInsBytes[3];

	uInsBytes[0] = (u8)(0x54|(uDir<<1)); 
	uInsBytes[1] = (u8)((uStAddr>>0)&0xff);
	uInsBytes[2] = (u8)((uStAddr>>8)&0xff);

	for(u32 i=0; i<3; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}
	m_uCodeOffset += 3;
}

/* DMAKILL (Kill) */
static void encodeDmaKill(void)
{
	u8 uInsBytes[1];
	
	uInsBytes[0] = (u8)(0x01);

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;
}

/* DMANOP (No operation) */
static void encodeDmaNop(void)
{
	u8 uInsBytes[1];
	
	uInsBytes[0] = (u8)(0x18);

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;
}


/* DMARMB (Read Memory Barrier) */
static void encodeDmaReadMemBarrier(void)
{
	u8 uInsBytes[1];
	
	uInsBytes[0] = (u8)(0x12);

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;
}

/* DMASEV (Send Event) */
static void encodeDmaSendEvent(u8 uEventNum) // 0~31
{
	assert(uEventNum < 32);

	u8 uInsBytes[2];
	
	uInsBytes[0] = (u8)(0x34);
	uInsBytes[1] = (u8)((uEventNum<<3)|0x0);

	for(u32 i=0; i<2; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 2;
}



/* DMAWMB (Write Memory Barrier) */
static void encodeDmaWriteMemBarrier(void)
{
	u8 uInsBytes[1];
	
	uInsBytes[0] = (u8)(0x13);

	for(u32 i=0; i<1; i++)
	{
		memOutp8(m_uCodeStAddr+m_uCodeOffset+i, uInsBytes[i]);
	}

	m_uCodeOffset += 1;
}


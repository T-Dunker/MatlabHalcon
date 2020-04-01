#include "mex.h"

#include <stdio.h>
#include <string.h>
#include <sstream>

#include <HALCONCpp/HalconCpp.h>

#ifdef WINDOWS
typedef unsigned __int64 UInt64;
#else
typedef unsigned long long UInt64;
#endif

using namespace HalconCpp;

int MatlabArray2HalconTuple(const mxArray *mxData, HTuple* hData)
{
  /* Check input image dimensions */
  mwSize ndimsD = mxGetNumberOfDimensions(mxData);
  if ((ndimsD < 1) || (ndimsD > 2)) { mexErrMsgTxt("Tuple Data must be 1D /2D"); }

  /* Get image dimensions*/
  size_t Ne = mxGetNumberOfElements(mxData);

  if (mxIsUint8(mxData))
  {
    unsigned char* DUint8 = (unsigned char*)mxGetData(mxData);
    for (int i = 0; i < Ne; i++) { (*hData)[i] = DUint8[i]; }
  }
  else if (mxIsInt16(mxData))
  {
    short* DInt16 = (short*)mxGetData(mxData);
    for (int i = 0; i < Ne; i++) { (*hData)[i] = DInt16[i]; }
  }
  else if (mxIsUint16(mxData))
  {
    unsigned short* DUint16 = (unsigned short*)mxGetData(mxData);
    for (int i = 0; i < Ne; i++) { (*hData)[i] = DUint16[i]; }
  }
  else if (mxIsInt32(mxData))
  {
    int* DInt32 = (int*)mxGetData(mxData);
    for (int i = 0; i < Ne; i++) { (*hData)[i] = DInt32[i]; }
  }
  else if (mxIsSingle(mxData))
  {
    float* DSingle = (float*)mxGetData(mxData);
    for (int i = 0; i < Ne; i++) { (*hData)[i] = DSingle[i]; }
  }
  else if (mxIsDouble(mxData))
  {
    double* DDouble = (double*)mxGetData(mxData);
    for (int i = 0; i < Ne; i++) { (*hData)[i] = DDouble[i]; }
  }
  else if (mxIsChar(mxData))
  {
    ((*hData)) = mxArrayToString(mxData);
  }
  else if (mxIsCell(mxData))
  {
    for (int i = 0; i < Ne; i++)
    {
      HTuple temp;
      mxArray * cData = mxGetCell(mxData, i);
      MatlabArray2HalconTuple(cData, &temp);
      (*hData)[i] = temp;
    }
  }
  else
  {
    mexErrMsgTxt("Tuple Type not supported");
  }

  return 0;
}

int HalconTuple2MatlabArray(HTuple* hData, mxArray **plhs)
{
  HTuple h_type, h_length;

  TupleType(*hData, &h_type);
  TupleLength(*hData, &h_length);

  int hv_type = h_type;
  int hv_length = h_length;

  int nDims = 2;
  mwSize dimsD[3];
  dimsD[0] = hv_length;
  dimsD[1] = 1;


  if (hv_type == 1) // Int
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsD, mxINT32_CLASS, mxREAL);
    int* dataInt32 = (int*)mxGetData(plhs[0]);
    for (int i = 0; i < hv_length; i++)
    {
      dataInt32[i] = (int)((*hData)[i]);
    }
  }
  else if (hv_type == 2) //real
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsD, mxDOUBLE_CLASS, mxREAL);
    double* dataDouble = (double*)mxGetData(plhs[0]);
    for (int i = 0; i < hv_length; i++)
    {
      dataDouble[i] = (double)((*hData)[i]);
    }
  }
  else if (hv_type == 4) //string
  {

    if (hv_length == 1)
    {
      HTuple h_charlength;
      TupleStrlen(*hData, &h_charlength);
      int hv_charlength = h_charlength;
      mwSize dimsC[2];
      dimsC[0] = 1;
      dimsC[1] = hv_charlength;
      plhs[0] = mxCreateCharArray(2, dimsC);
      mxChar* dataChar = (mxChar *)mxGetData(plhs[0]);
      const char* str = (*hData)[0];
      for (int i = 0; i < hv_charlength; i++)
      {
        dataChar[i] = str[i];
      }
    }
    else
    {
      // Multiple strings, we need a cell array (as in mixed)
      hv_type = 8;
    }
  }
  if (hv_type == 8) //mixed
  {
    plhs[0] = mxCreateCellArray(nDims, dimsD);
    for (int i = 0; i < hv_length; i++)
    {
      HTuple hVal = (*hData)[i];
      mxArray* value;
      HalconTuple2MatlabArray(&hVal, &value);
      mxSetCell(plhs[0], i, value);
    }
  }
  else if (hv_type == 15) //empty - Any
  {
    mwSize dimsE[3];
    dimsE[0] = 0;
    dimsE[1] = 0;
    plhs[0] = mxCreateNumericArray(2, dimsE, mxDOUBLE_CLASS, mxREAL);
  }

  return 0;
}


int MatlabArray2HalconImage(const mxArray *mxImage, HObject* hImagep)
{

  /* Check input image dimensions */
  mwSize ndimsI = mxGetNumberOfDimensions(mxImage);
  if ((ndimsI < 2) || (ndimsI > 3)) { mexErrMsgTxt("Image must be 2D"); }

  /* Get image dimensions*/
  const mwSize *dimsI;
  dimsI = mxGetDimensions(mxImage);

  // Convert pointer to Halcon image
  HTuple width = (int)dimsI[0];
  HTuple height = (int)dimsI[1];
  mwSize npix = dimsI[0] * dimsI[1];
  int nChannels = 1;
  if (ndimsI == 3) { nChannels = (int)dimsI[2]; }

  // Detect type
  if (mxIsUint8(mxImage))
  {
    unsigned char* IUint8 = (unsigned char*)mxGetData(mxImage);
    if (nChannels == 1)
    {
      HalconCpp::GenImage1(hImagep, "byte", width, height, (Hlong)IUint8);
    }
    else if (nChannels == 3)
    {
      HalconCpp::GenImage3(hImagep, "byte", width, height, (Hlong)IUint8, (Hlong)&IUint8[npix], (Hlong)&IUint8[npix * 2]);
    }
    else
    {
      HObject hImaget;
      HalconCpp::GenImage1(&hImaget, "byte", width, height, (Hlong)IUint8);
      for (int i = 1; i < nChannels; i++)
      {
        HObject hImageg;
        HalconCpp::GenImage1(&hImageg, "byte", width, height, (Hlong)&IUint8[i*npix]);
        HalconCpp::ConcatObj(hImaget, hImageg, &hImaget);
      }
      HalconCpp::ChannelsToImage(hImaget, hImagep);
    }
  }
  else if (mxIsInt16(mxImage))
  {
    short* IInt16 = (short*)mxGetData(mxImage);
    if (nChannels == 1)
    {
      HalconCpp::GenImage1(hImagep, "int2", width, height, (Hlong)IInt16);
    }
    else if (nChannels == 3)
    {
      HalconCpp::GenImage3(hImagep, "int2", width, height, (Hlong)IInt16, (Hlong)&IInt16[npix], (Hlong)&IInt16[npix * 2]);
    }
    else
    {
      HObject hImaget;
      HalconCpp::GenImage1(&hImaget, "int2", width, height, (Hlong)IInt16);
      for (int i = 1; i < nChannels; i++)
      {
        HObject hImageg;
        HalconCpp::GenImage1(&hImageg, "int2", width, height, (Hlong)&IInt16[i*npix]);
        HalconCpp::ConcatObj(hImaget, hImageg, &hImaget);
      }
      HalconCpp::ChannelsToImage(hImaget, hImagep);
    }
  }
  else if (mxIsUint16(mxImage))
  {
    unsigned short* IUint16 = (unsigned short*)mxGetData(mxImage);
    if (nChannels == 1)
    {
      HalconCpp::GenImage1(hImagep, "uint2", width, height, (Hlong)IUint16);
    }
    else if (nChannels == 3)
    {
      HalconCpp::GenImage3(hImagep, "uint2", width, height, (Hlong)IUint16, (Hlong)&IUint16[npix], (Hlong)&IUint16[npix * 2]);
    }
    else
    {
      HObject hImaget;
      HalconCpp::GenImage1(&hImaget, "uint2", width, height, (Hlong)IUint16);
      for (int i = 1; i < nChannels; i++)
      {
        HObject hImageg;
        HalconCpp::GenImage1(&hImageg, "uint2", width, height, (Hlong)&IUint16[i*npix]);
        HalconCpp::ConcatObj(hImaget, hImageg, &hImaget);
      }
      HalconCpp::ChannelsToImage(hImaget, hImagep);
    }
  }
  else if (mxIsInt32(mxImage))
  {
    int* IInt32 = (int*)mxGetData(mxImage);
    if (nChannels == 1)
    {
      HalconCpp::GenImage1(hImagep, "int4", width, height, (Hlong)IInt32);
    }
    else if (nChannels == 3)
    {
      HalconCpp::GenImage3(hImagep, "int4", width, height, (Hlong)IInt32, (Hlong)&IInt32[npix], (Hlong)&IInt32[npix * 2]);
    }
    else
    {
      HObject hImaget;
      HalconCpp::GenImage1(&hImaget, "int4", width, height, (Hlong)IInt32);
      for (int i = 1; i < nChannels; i++)
      {
        HObject hImageg;
        HalconCpp::GenImage1(&hImageg, "int4", width, height, (Hlong)&IInt32[i*npix]);
        HalconCpp::ConcatObj(hImaget, hImageg, &hImaget);
      }
      HalconCpp::ChannelsToImage(hImaget, hImagep);
    }
  }
  else if (mxIsSingle(mxImage))
  {
    float* ISingle = (float*)mxGetData(mxImage);
    if (nChannels == 1)
    {
      HalconCpp::GenImage1(hImagep, "real", width, height, (Hlong)ISingle);
    }
    else if (nChannels == 3)
    {
      HalconCpp::GenImage3(hImagep, "real", width, height, (Hlong)ISingle, (Hlong)&ISingle[npix], (Hlong)&ISingle[npix * 2]);
    }
    else
    {
      HObject hImaget;
      HalconCpp::GenImage1(&hImaget, "real", width, height, (Hlong)ISingle);
      for (int i = 1; i < nChannels; i++)
      {
        HObject hImageg;
        HalconCpp::GenImage1(&hImageg, "real", width, height, (Hlong)&ISingle[i*npix]);
        HalconCpp::ConcatObj(hImaget, hImageg, &hImaget);
      }
      HalconCpp::ChannelsToImage(hImaget, hImagep);
    }
  }
  else
  {
    mexErrMsgTxt("Image Array type not supported");
  }
  return 0;
}

int MatlabArray2HalconRegion(const mxArray *mxRegion, HObject* hRegionp)
{
  /* Check input image dimensions */
  mwSize ndimsR = mxGetNumberOfDimensions(mxRegion);
  if (ndimsR != 2) { mexErrMsgTxt("Region must be 2D"); }

  /* Get region dimensions*/
  const mwSize *dimsR;
  dimsR = mxGetDimensions(mxRegion);

  if (dimsR[1] != 3)
  {
    mexErrMsgTxt("Region size must be n x 3 ");
  }


  // Local control variables 
  HTuple  hv_Row, hv_ColumnBegin, hv_ColumnEnd;

  //   TupleGenConst(5, 0, &hv_r);
  hv_Row.Clear();
  hv_ColumnBegin.Clear();
  hv_ColumnEnd.Clear();

  if (mxIsInt32(mxRegion))
  {
    int* regpoints_i = (int*)mxGetData(mxRegion);
    for (int i = 0; i < dimsR[0]; i++)
    {
      hv_Row[i] = regpoints_i[i];
      hv_ColumnBegin[i] = regpoints_i[i + dimsR[0]];
      hv_ColumnEnd[i] = regpoints_i[i + dimsR[0] * 2];
    }
  }
  else if (mxIsDouble(mxRegion))
  {
    double* regpoints_d = (double*)mxGetData(mxRegion);
    for (int i = 0; i < dimsR[0]; i++)
    {
      hv_Row[i] = (int)regpoints_d[i];
      hv_ColumnBegin[i] = (int)regpoints_d[i + dimsR[0]];
      hv_ColumnEnd[i] = (int)regpoints_d[i + dimsR[0] * 2];
    }
  }
  else
  {
    mexErrMsgTxt("Region Array type not supported");
  }
  GenRegionRuns(hRegionp, hv_Row, hv_ColumnBegin, hv_ColumnEnd);
  return 0;
}

int MatlabArray2HalconXLD(const mxArray *input, HObject* hXld)
{
  HSerializedItem item(mxGetData(input), mxGetNumberOfElements(input), "false");
  hXld->DeserializeObject(item);
  return 0;
}

int MatlabArray2HalconObject(const mxArray *input, HObject* hImagep)
{
  try
  {
    // If struct unwrap
    // a.image = m x n 
    // a.region = o x 3
    // a.xld = k x 1
    if (mxIsStruct(input))
    {

      int nfields = mxGetNumberOfFields(input);
      int NStructElems = (int)mxGetNumberOfElements(input);
      for (int index = 0; index < NStructElems; index++)
      {
        const mxArray *mxImage = 0;
        const mxArray *mxRegion = 0;
        const mxArray *mxXld = 0;

        for (int ifield = 0; ifield < nfields; ifield++)
        {
          mxArray *tmp = mxGetFieldByNumber(input, index, ifield);
          const char *fname = mxGetFieldNameByNumber(input, ifield);

          if (strcmp(fname, "image") == 0)
          {
            if (tmp != NULL ? !mxIsEmpty(tmp) : false)
            {
              mxImage = tmp;
            }
          }
          if (strcmp(fname, "region") == 0)
          {
            if (tmp != NULL ? !mxIsEmpty(tmp) : false)
            {
              mxRegion = tmp;
            }
          }
          if (strcmp(fname, "xld") == 0)
          {
            if (tmp != NULL ? !mxIsEmpty(tmp) : false)
            {
              mxXld = tmp;
            }
          }
        }

        if ((mxImage == 0) && (mxRegion == 0) && (mxXld == 0))
        {
          mexErrMsgTxt("No Input Image, Region or XLD Found");
        }

        // Convert Matlab Image array to halcon image
        if (index == 0)
        {
          if (mxXld != 0)
          {
            MatlabArray2HalconXLD(mxXld, hImagep);
          }
          if (mxImage != 0)
          {
            MatlabArray2HalconImage(mxImage, hImagep);
          }
          if (mxRegion != 0)
          {
            // If Matlab region array exist reduce region of current image
            HObject hRegion;
            if (mxImage != 0)
            {
              MatlabArray2HalconRegion(mxRegion, &hRegion);
              ReduceDomain(hImagep[0], hRegion, hImagep);
            }
            else
            {
              MatlabArray2HalconRegion(mxRegion, hImagep);
            }
          }
        }
        else
        {
          HObject hImaget;
          if (mxXld != 0)
          {
            MatlabArray2HalconXLD(mxXld, &hImaget);
          }
          if (mxImage != 0)
          {
            MatlabArray2HalconImage(mxImage, &hImaget);
          }
          if (mxRegion != 0)
          {
            // If Matlab region array exist reduce region of current image
            HObject hRegion;
            if (mxImage != 0)
            {
              MatlabArray2HalconRegion(mxRegion, &hRegion);
              ReduceDomain(hImaget, hRegion, &hImaget);
            }
            else
            {
              MatlabArray2HalconRegion(mxRegion, &hImaget);
            }
          }

          ConcatObj(*hImagep, hImaget, hImagep);


        }

      }
    }
    else
    {
      MatlabArray2HalconImage(input, hImagep);
    }
  }
  catch (HalconCpp::HException &HDevExpDefaultException)
  {
    mexErrMsgTxt(HDevExpDefaultException.ErrorMessage().Text());
  }
  return 0;
}

int HalconXLD2MatlabArray(HObject* hXld, mxArray **plhs)
{
  HSerializedItem item = hXld->SerializeObject();
  Hlong sz(0);
  void* pData = item.GetSerializedItemPtr(&sz);
  plhs[0] = mxCreateNumericMatrix(sz, 1, mxUINT8_CLASS, mxREAL);
  memcpy(mxGetData(plhs[0]), pData, sz);
  return 0;
}

int HalconImage2MatlabArray(HObject* hImagep, mxArray **plhs)
{
  // Local control variables 
  HTuple  hv_Pointer, hv_Type, hv_Width, hv_Height;

  // Local control variables 
  HTuple hv_Channels;
  //HTuple hv_Objects;

  //CountObj(hImagep[0], &hv_Objects);
  CountChannels(hImagep[0], &hv_Channels);

  int nChannels = hv_Channels;

  GetImagePointer1(hImagep[0], &hv_Pointer, &hv_Type, &hv_Width, &hv_Height);
  mwSize dimsI[3];
  dimsI[0] = hv_Width.I();
  dimsI[1] = hv_Height.I();
  dimsI[2] = nChannels;
  mwSize npix = dimsI[0] * dimsI[1];
  int nDims = 2;
  if (nChannels > 1) { nDims = 3; }
  mwSize ndata = 0;

  if (hv_Type == "byte")
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsI, mxUINT8_CLASS, mxREAL);
    ndata = npix;
  }
  else if (hv_Type == "int2")
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsI, mxINT16_CLASS, mxREAL);
    ndata = npix * sizeof(short);
  }
  else if (hv_Type == "uint2")
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsI, mxUINT16_CLASS, mxREAL);
    ndata = npix * sizeof(unsigned short);
  }
  else if (hv_Type == "int4")
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsI, mxINT32_CLASS, mxREAL);
    ndata = npix * sizeof(int);
  }
  else if (hv_Type == "real")
  {
    plhs[0] = mxCreateNumericArray(nDims, dimsI, mxSINGLE_CLASS, mxREAL);
    ndata = npix * sizeof(float);
  }

  if (nChannels == 1)
  {
    memcpy((void*)mxGetData(plhs[0]), (void *)((Hlong)hv_Pointer), ndata);
  }
  else
  {
    for (int i = 0; i < nChannels; i++)
    {
      HObject himage_t;
      AccessChannel(hImagep[0], &himage_t, i + 1);
      GetImagePointer1(himage_t, &hv_Pointer, &hv_Type, &hv_Width, &hv_Height);
      memcpy((void*)((byte*)mxGetData(plhs[0]) + i * ndata), (void *)((Hlong)hv_Pointer), ndata);
    }
  }

  return 0;
}

int HalconRegion2MatlabArray(HObject* ho_region, mxArray **plhs)
{
  // Get region runs
  HTuple  hv_Rows, hv_Columns1, hv_Columns2;
  GetRegionRuns(ho_region[0], &hv_Rows, &hv_Columns1, &hv_Columns2);

  // Copy region runs to matlab array with runs
  mwSize dimsR[2];
  dimsR[0] = (int)hv_Rows.TupleLength();
  dimsR[1] = 3;
  plhs[0] = mxCreateNumericArray(2, dimsR, mxINT32_CLASS, mxREAL);
  int* data = (int*)mxGetData(plhs[0]);
  for (int i = 0; i < dimsR[0]; i++)
  {
    data[i] = (int)((const HTuple&)hv_Rows)[i];
    data[i + dimsR[0]] = (int)((const HTuple&)hv_Columns1)[i];
    data[i + dimsR[0] * 2] = (int)((const HTuple&)hv_Columns2)[i];
  }
  return 0;
}




int HalconObject2MatlabArray(HObject* hImagep, mxArray **plhs)
{
  try
  {
    HTuple hv_Objects;
    CountObj(hImagep[0], &hv_Objects);
    int nObj = (int)hv_Objects;
    // Create Output structs
    int nfields = 3;
    const char *fieldnames[] = { "image", "region", "xld" };
    plhs[0] = mxCreateStructMatrix(nObj, 1, nfields, fieldnames);

    // Loop through all image objects
    for (int i = 0; i < nObj; i++)
    {
      /*
      for (int j = 0; j < nfields; ++j)
      {
        mxSetFieldByNumber(plhs[0], i, j, 0);
      }
      */
      HObject  hImaget;

      SelectObj(hImagep[0], &hImaget, i + 1);
      // mexPrintf("Class %s\n", hImaget.GetObjClass().S().Text());
      size_t maxCount = hImaget.GetObjClass().S().Length();
      maxCount = maxCount < 3 ? maxCount : 3;
      if (strncmp("xld", hImaget.GetObjClass().S().Text(), maxCount) == 0)
      {
        mxArray *Xout;
        HalconXLD2MatlabArray(&hImaget, &Xout);
        // Set struct Field image
        mxSetFieldByNumber(plhs[0], i, 2, Xout);
        continue;
      }

      // Local control variables 
      HTuple hv_Channels;
      CountChannels(hImaget, &hv_Channels);
      int nChannels = hv_Channels;

      // If we have image data convert image
      if (nChannels > 0)
      {
        mxArray *Iout;
        HalconImage2MatlabArray(&hImaget, &Iout);
        // Set struct Field image
        mxSetFieldByNumber(plhs[0], i, 0, Iout);
      }

      // Convert Region
      mxArray *Rout;
      HalconRegion2MatlabArray(&hImaget, &Rout);
      // Set struct Field region
      mxSetFieldByNumber(plhs[0], i, 1, Rout);
    }
  }
  catch (HalconCpp::HException &HDevExpDefaultException)
  {
    mexErrMsgTxt(HDevExpDefaultException.ErrorMessage().Text());
  }
  return 0;
}


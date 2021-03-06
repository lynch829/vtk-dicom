/*=========================================================================

  Program: DICOM for VTK

  Copyright (c) 2012-2015 David Gobbi
  All rights reserved.
  See Copyright.txt or http://dgobbi.github.io/bsd3.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDICOMReader - Read DICOM image files.
// .SECTION Description
// This class reads a series of DICOM files into a vtkImageData object,
// and also provides access to the DICOM meta data for each file.

#ifndef vtkDICOMReader_h
#define vtkDICOMReader_h

#include <vtkImageReader2.h>
#include "vtkDICOMModule.h" // For export macro

class vtkIntArray;
class vtkTypeInt64Array;
class vtkStringArray;
class vtkMatrix4x4;
class vtkMedicalImageProperties;
class vtkDICOMMetaData;
class vtkDICOMParser;
class vtkDICOMSliceSorter;

//----------------------------------------------------------------------------
class VTKDICOM_EXPORT vtkDICOMReader : public vtkImageReader2
{
public:
  vtkTypeMacro(vtkDICOMReader, vtkImageReader2);

  // Description:
  // Static method for construction.
  static vtkDICOMReader *New();

  // Description:
  // Print information about this object.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Valid extensions for this file type.
  virtual const char* GetFileExtensions() {
    return ".dcm .dc"; }

  // Description:
  // Return a descriptive name that might be useful in a GUI.
  virtual const char* GetDescriptiveName() {
    return "DICOM"; }

  // Description:
  // Return true if this reader can read the given file.
  int CanReadFile(const char* filename);

  // Description:
  // Set the Stack ID of the stack to load, for named stacks.
  // If the series has multiple stacks, then by default the reader
  // will only load the first stack.  This method allows you to select
  // a different stack, if you know the DICOM StackID for the stack.
  void SetDesiredStackID(const char *stackId);
  const char *GetDesiredStackID() { return this->DesiredStackID; }

  // Description:
  // Get a list of the stacks that are present in the input files.
  // A stack is a contiguous array of slices that form a volume.
  vtkStringArray *GetStackIDs() { return this->StackIDs; }

  // Description:
  // Get an array that converts slice index to input file index.
  // If the reader has generated scalar components, then this will
  // be a two-dimensional array and calling array->GetComponent(i,j)
  // will return the file index for slice i and scalar component j
  // for monochrome images, or for slice i and scalar component 3*j
  // for RGB images (or more precisely, at scalar component N*j where
  // N is the SamplesPerPixel value from the DICOM metadata).  If the
  // data has just one component, then use j=0. If you used SetFileNames()
  // to provide a list of files to the reader, then use this array to
  // find out which file provided which slice, or to index into the
  // MetaData object to get the metadata for a particular slice.
  vtkIntArray *GetFileIndexArray() { return this->FileIndexArray; }

  // Description:
  // Get an array that converts slice index to frame index.
  // The purpose of this array is to identify individual frames in
  // multi-frame DICOM files.  The dimensions of this array are identical
  // to the FileIndexArray.  Use FileIndexArray to identify the file,
  // then use FrameIndexArray to identify the frame within that file.
  vtkIntArray *GetFrameIndexArray() { return this->FrameIndexArray; }

  // Description:
  // Get the meta data for the DICOM files.
  // The GetAttributeValue() method of vtkDICOMMataData takes optional
  // file and frame indices, which specify the file and the frame within
  // that file to get the attribute from.  If you have a slice index rather
  // than a file index and frame index, then use the FileIndexArray and
  // FrameIndexArray to convert the slice index into file and frame indices.
  vtkDICOMMetaData *GetMetaData() { return this->MetaData; }

  // Description:
  // If the files have been pre-sorted, the sorting can be disabled.
  vtkGetMacro(Sorting, int);
  vtkSetMacro(Sorting, int);
  vtkBooleanMacro(Sorting, int)

  // Description:
  // Set a custom sorter to be used to sort files and frames into slices.
  // The default sorter uses the attributes "ImagePositionPatient" and
  // "ImageOrientationPatient" to spatially arrange the slices.
  void SetSorter(vtkDICOMSliceSorter *sorter);
  vtkDICOMSliceSorter *GetSorter() { return this->Sorter; }

  // Description:
  // Read the time dimension as scalar components (default: Off).
  // If this is on, then each time point will be stored as a scalar
  // component in the image data.  If the data has both a time dimension
  // and a vector dimension, then the number of components will be the
  // product of these two dimensions, i.e. the components will store
  // a sequence of vectors, one vector for each time point.
  vtkGetMacro(TimeAsVector, int);
  vtkSetMacro(TimeAsVector, int);
  vtkBooleanMacro(TimeAsVector, int);

  // Description:
  // Get the time dimension if the DICOM series has one.
  int GetTimeDimension() { return this->TimeDimension; }
  double GetTimeSpacing() { return this->TimeSpacing; }

  // Description:
  // Set the desired time index (set to -1 for all).
  vtkSetMacro(DesiredTimeIndex, int);
  vtkGetMacro(DesiredTimeIndex, int);

  // Description:
  // Turn off automatic conversion of YBR images to RGB.
  // By default, YBR images are always converted to RGB (though the
  // photometric interpretation in the metadata will remain the same).
  vtkGetMacro(AutoYBRToRGB, int);
  vtkSetMacro(AutoYBRToRGB, int);
  vtkBooleanMacro(AutoYBRToRGB, int);

  // Description:
  // Turn off automatic rescaling of intensity values.
  // By default, of the reader sees different RescaleSlope and
  // RescaleIntercept values for different slices, then it will
  // adjust the pixel values so that they can all use the same
  // slope and intercept.  This is a lossy process, so you might
  // want to turn it off and use vtkDICOMApplyRescale instead.
  vtkGetMacro(AutoRescale, int);
  vtkSetMacro(AutoRescale, int);
  vtkBooleanMacro(AutoRescale, int);

  // Description:
  // Get the slope and intercept for rescaling the scalar values.
  // These values allow calibration of the data to real values.
  // Use the equation v = u*RescaleSlope + RescaleIntercept.
  double GetRescaleSlope() { return this->RescaleSlope; }
  double GetRescaleIntercept() { return this->RescaleIntercept; }

  // Description:
  // Get a matrix to place the image within DICOM patient coords.
  // This matrix is constructed from the ImageOrientationPatient
  // and ImagePositionPatient meta data attributes.  See the
  // SetMemoryRowOrder method for additional information.
  vtkMatrix4x4 *GetPatientMatrix() { return this->PatientMatrix; }

  // Description:
  // Get a MedicalImageProperties object for this file.
  vtkMedicalImageProperties *GetMedicalImageProperties();

  // Description:
  // Enumeration for top-down vs. bottom-up ordering.
  enum RowOrder { FileNative, TopDown, BottomUp };

  // Description:
  // Set the ordering of the image rows in memory.
  // If the order is BottomUp (which is the default) then
  // the images will be flipped when they are read from disk.
  // The native orientation of DICOM images is top-to-bottom.
  void SetMemoryRowOrder(int order);
  void SetMemoryRowOrderToFileNative() {
    this->SetMemoryRowOrder(FileNative); }
  void SetMemoryRowOrderToTopDown() {
    this->SetMemoryRowOrder(TopDown); }
  void SetMemoryRowOrderToBottomUp() {
    this->SetMemoryRowOrder(BottomUp); }
  int GetMemoryRowOrder() { return this->MemoryRowOrder; }
  const char *GetMemoryRowOrderAsString();

protected:
  vtkDICOMReader();
  ~vtkDICOMReader();

  // Description:
  // Read the header information.
  virtual int RequestInformation(
    vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector);

  // Description:
  // Read the voxel data.
  virtual int RequestData(
    vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector);

  // Description:
  // Read one file.  Specify the offset to the PixelData.
  virtual bool ReadOneFile(
    const char *filename, int idx,
    unsigned char *buffer, vtkIdType bufferSize);

  // Description:
  // Unpack 1 bit to 8 bits or 12 bits to 16 bits.
  void UnpackBits(
    const void *source, void *buffer, vtkIdType bufferSize, int bits);

  // Description:
  // Read an DICOM file directly.
  virtual bool ReadFileNative(
    const char *filename, int idx,
    unsigned char *buffer, vtkIdType bufferSize);

  // Description:
  // Read a DICOM file via DCMTK or GDCM.
  virtual bool ReadFileDelegated(
    const char *filename, int idx,
    unsigned char *buffer, vtkIdType bufferSize);

  // Description:
  // Rescale the data in the buffer.
  virtual void RescaleBuffer(
    int fileIdx, int frameIdx, void *buffer, vtkIdType bufferSize);

  // Description:
  // Convert buffer from YUV to RGB.
  virtual void YBRToRGB(
    int fileIdx, int frameIdx, void *buffer, vtkIdType bufferSize);

  // Description:
  // Convert parser errors into reader errors.
  void RelayError(vtkObject *o, unsigned long e, void *data);

  // Description:
  // Verify that the files can be composed into a volume.
  // This is called after SortFiles has been called.  It should invoke an
  // error event and return zero upon failure.
  virtual bool ValidateStructure(
    vtkIntArray *fileArray, vtkIntArray *frameArray);

  // Description:
  // Sort the input files, put the sort in the supplied arrays.
  virtual void SortFiles(vtkIntArray *fileArray, vtkIntArray *frameArray);

  // Description:
  // Do not sort the files, just build the arrays.
  void NoSortFiles(vtkIntArray *fileArray, vtkIntArray *frameArray);

  // Description:
  // Update the medical image properties;
  virtual void UpdateMedicalImageProperties();

  // Description:
  // Select whether to sort the files.
  int Sorting;

  // Description:
  // Information for rescaling data to quantitative units.
  double RescaleIntercept;
  double RescaleSlope;

  // Description:
  // The orientation matrix for the DICOM file.
  vtkMatrix4x4 *PatientMatrix;

  // Description:
  // The meta data for the image.
  vtkDICOMMetaData *MetaData;

  // Description:
  // The MedicalImageProperties, for compatibility with other readers.
  vtkMedicalImageProperties *MedicalImageProperties;

  // Description:
  // The parser that is used to read the file.
  vtkDICOMParser *Parser;

  // Description:
  // The sorter that orders the slices within the volume.
  vtkDICOMSliceSorter *Sorter;

  // Description:
  // The offsets to the pixel data in each file.
  vtkTypeInt64Array *FileOffsetArray;

  // Description:
  // An array to convert slice indices to input files
  vtkIntArray *FileIndexArray;

  // Description:
  // An array to convert slice indices to input frames
  vtkIntArray *FrameIndexArray;

  // Description:
  // An array that holds the stack IDs.
  vtkStringArray *StackIDs;

  // Description:
  // The row order to use when storing the data in memory.
  int MemoryRowOrder;

  // Description:
  // This indicates that the data must be rescaled.
  int NeedsRescale;
  int AutoRescale;

  // Description:
  // This indicates that the data must be converted to RGB.
  int NeedsYBRToRGB;
  int AutoYBRToRGB;

  // Description:
  // The number of packed pixel components in the input file.
  // This is for packed, rather than planar, components.
  int NumberOfPackedComponents;

  // Description:
  // The number of color planes in the file.
  int NumberOfPlanarComponents;

  // Description:
  // Time dimension variables.
  int TimeAsVector;
  int TimeDimension;
  int DesiredTimeIndex;
  double TimeSpacing;

  // Description:
  // The stack to load.
  char DesiredStackID[20];

private:
  vtkDICOMReader(const vtkDICOMReader&);  // Not implemented.
  void operator=(const vtkDICOMReader&);  // Not implemented.
};

#endif // vtkDICOMReader_h

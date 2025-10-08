cmake_minimum_required(VERSION 3.26)

project(OpenFOAM-TT VERSION "1.0.0" LANGUAGES CXX)

set(OPENFOAM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/OpenFOAM-dev/src)
set(OPENFOAM_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/OpenFOAM-dev/platforms/linux64GccSPInt32Opt/lib)

option(DAISY_INSTRUMENTATION "Enable Daisy instrumentation" OFF)

option(ENABLE_TT "Enable Manual Tenstorrent offloading" OFF)
option(VERIFY_TT "Enable verification of Tenstorrent offloading results" ON)

set(OPENFOAM_OS_SPECIFIC_SOURCES
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/signals/sigFpe.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/signals/sigSegv.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/signals/sigInt.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/signals/sigQuit.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/signals/sigStopAtWriteNow.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/signals/sigWriteNow.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/regExp.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/timer.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/fileStat.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/POSIX.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/cpuTime/cpuTime.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/clockTime/clockTime.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/memInfo/memInfo.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/fileMonitor.C
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/printStack.C
)

set(OPENFOAM_PSTREAM_SOURCES
  ${OPENFOAM_SRC_DIR}/Pstream/dummy/UPstream.C
  ${OPENFOAM_SRC_DIR}/Pstream/dummy/UIPread.C
  ${OPENFOAM_SRC_DIR}/Pstream/dummy/UOPwrite.C
)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/global.Cver.C
  COMMAND sed -e 's!BUILD_STRING!-!' -e 's!VERSION_STRING!unknown!' ${OPENFOAM_SRC_DIR}/OpenFOAM/global/global.Cver > ${CMAKE_BINARY_DIR}/global.Cver.C
  DEPENDS ${OPENFOAM_SRC_DIR}/OpenFOAM/global/global.Cver
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating global.Cver.C"
)

set(TT_SOURCES
  src/tenstorrent/device_transfers.cpp
  src/tenstorrent/kernel_launcher.cpp
  src/tenstorrent/ldu_meta_cache.cpp
)


add_library(OpenFOAM SHARED
  ${OPENFOAM_OS_SPECIFIC_SOURCES}
  ${CMAKE_BINARY_DIR}/global.Cver.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/argList/argList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/clock/clock.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/etcFiles/etcFiles.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/fileOperation/fileOperation.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/fileOperationInitialise/fileOperationInitialise.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/uncollatedFileOperation/uncollatedFileOperation.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/masterUncollatedFileOperation/masterUncollatedFileOperation.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/collatedFileOperation/collatedFileOperation.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/collatedFileOperation/hostCollatedFileOperation.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/collatedFileOperation/threadedCollatedOFstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/fileOperations/collatedFileOperation/OFstreamCollator.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/bools/bool/bool.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/bools/bool/boolIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/bools/Switch/Switch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/bools/Switch/SwitchIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/bools/lists/boolList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/chars/char/charIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/chars/wchar/wcharIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/direction/directionIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/uint32/uint32.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/uint32/uint32IO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/uint64/uint64.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/uint64/uint64IO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/uint/uintIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/int32/int32.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/int32/int32IO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/int64/int64.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/int64/int64IO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/int/intIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/label/label.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/uLabel/uLabel.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/lists/labelIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ints/lists/labelListIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/longDoubleScalar/longDoubleScalar.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/doubleScalar/doubleScalar.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/floatScalar/floatScalar.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/scalar/scalar.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/scalar/incGamma.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/scalar/invIncGamma.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/lists/scalarList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/lists/scalarIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Scalar/lists/scalarListIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/DiagTensor/diagTensor/diagTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/SphericalTensor/sphericalTensor/sphericalTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/SphericalTensor/labelSphericalTensor/labelSphericalTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/SymmTensor/labelSymmTensor/labelSymmTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/SymmTensor/symmTensor/symmTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Tensor/labelTensor/labelTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Tensor/tensor/tensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Tensor/lists/sphericalTensorList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Tensor/lists/symmTensorList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Tensor/lists/tensorList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector/complexVector/complexVector.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector/labelVector/labelVector.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector/vector/vector.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector/lists/vectorList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector/lists/vectorIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector/lists/vectorListIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Tensor2D/tensor2D/tensor2D.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/SphericalTensor2D/sphericalTensor2D/sphericalTensor2D.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/SymmTensor2D/symmTensor2D/symmTensor2D.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Vector2D/vector2D/vector2D.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/complex/complex.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/globalIndexAndTransform/globalIndexAndTransform.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/transform/transformer/transformer.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/transform/transformer/transformerIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/quaternion/quaternion.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/septernion/septernion.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/triad/triad.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/omega/omega.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/Function1/Function1UnitConversions.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/makeFunction1s.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/reverseRamp/reverseRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/linearRamp/linearRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/quadraticRamp/quadraticRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/quarterSineRamp/quarterSineRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/quarterCosineRamp/quarterCosineRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/halfCosineRamp/halfCosineRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/exponentialSqrRamp/exponentialSqrRamp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/normalise/normalise.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/squarePulse/squarePulse.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/Table/tableBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/Table/TableReader/makeTableReaders.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/unknownTypeFunction1/unknownTypeFunction1.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/omega1/omega1.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function1/Table/TableReader/Foam/tokenTupleN.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function2/Function2/Function2UnitConversions.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/functions/Function2/makeFunction2s.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/string/string.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/string/stringIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/string/stringIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/verbatimString/verbatimString.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/verbatimString/verbatimStringIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/word/word.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/word/wordIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/word/wordIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/fileName/fileName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/fileName/fileNameIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/variable/variable.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/variable/variableIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/functionName/functionName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/functionName/functionNameIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/wordRe/wordRe.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/lists/hashedWordList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/stringOps/stringOps.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ops/flipOp.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/hashes/Hasher/Hasher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/hashes/SHA1/SHA1.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/hashes/SHA1/SHA1Digest.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/randomGenerator/randomGenerator.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ranges/labelRange/labelRange.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ranges/labelRange/labelRanges.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ranges/scalarRange/scalarRange.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/ranges/scalarRange/scalarRanges.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/spatialVectorAlgebra/SpatialVector/spatialVector/spatialVector.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/spatialVectorAlgebra/SpatialTensor/spatialTensor/spatialTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/spatialVectorAlgebra/CompactSpatialTensor/compactSpatialTensor/compactSpatialTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/polynomialEqns/cubicEqn/cubicEqn.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/polynomialEqns/quadraticEqn/quadraticEqn.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Barycentric/barycentric/barycentric.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/Barycentric2D/barycentric2D/barycentric2D.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/HashTables/HashTable/HashTableCore.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/HashTables/ListHashTable/ListHashTableCore.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/Lists/SortableList/ParSortableListName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/Lists/PackedList/PackedListCore.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/Lists/PackedList/PackedBoolList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/Lists/ListOps/ListOps.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/LinkedLists/linkTypes/SLListBase/SLListBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/LinkedLists/linkTypes/DLListBase/DLListBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/token/tokenIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams/IOstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams/Istream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams/Ostream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/ISstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/OSstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/SstreamsPrint.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/readHexLabel.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/prefixOSstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/gzstream/gzstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Fstreams/IFstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Fstreams/OFstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Fstreams/masterOFstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Tstreams/ITstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/StringStreams/StringStreamsPrint.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/UIPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/IPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/UPstreamCommsStruct.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/Pstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/UOPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/OPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/PstreamBuffers.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/keyType/keyType.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/entry/entry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/entry/entryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/primitiveEntry/primitiveEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/primitiveEntry/primitiveEntryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryEntry/dictionaryEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryEntry/dictionaryEntryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryListEntry/dictionaryListEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryListEntry/dictionaryListEntryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/mergeDictionaries/mergeDictionaries.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/negEntry/negEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/calcIncludeEntry/calcIncludeEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/calcEntry/calcEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/codeStream/codeStream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/functionEntry/functionEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/includeEntry/includeEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/includeEtcEntry/includeEtcEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/includeFuncEntry/includeFuncEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/includeIfPresentEntry/includeIfPresentEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/inputModeEntry/inputModeEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/removeEntry/removeEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/ifeqEntry/ifeqEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/ifEntry/ifEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/IOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/IOdictionaryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/localIOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/timeIOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/systemDict.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/decomposedBlockData/decomposedBlockData.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobject/IOobject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobject/IOobjectIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobject/IOobjectReadHeader.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobject/IOobjectWriteHeader.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/regIOobject/regIOobject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/regIOobject/regIOobjectRead.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/regIOobject/regIOobjectWrite.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjectList/IOobjectList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/objectRegistry/objectRegistry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/CallbackRegistry/CallbackRegistryName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/dlLibraryTable/dlLibraryTable.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/dynamicCode/dynamicCode.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/dynamicCode/dynamicCodeContext.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/codedBase/codedBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/compileTemplate/compileTemplate.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/functionObject/functionObject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/functionObjectList/functionObjectList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/functionObjectList/wordAndDictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/writeFile/writeFile.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/logFiles/logFiles.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/logFile/logFile.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/writeObjectsBase/writeObjectsBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/writeLocalObjects/writeLocalObjects.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/timeControl/timeControl.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/timeControl/timeControlFunctionObject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/objectRegistryFunctionObject/objectRegistryFunctionObject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/regionFunctionObject/regionFunctionObject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/controlIOdictionary/controlIOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/TimePaths.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/TimeState.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/Time.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/TimeIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/findTimes.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/subCycleTime.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/timeSelector.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/instant/instant.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/userTime/userTime/userTime.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/userTime/userTime/userTimeNew.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/userTime/realTime/realTime.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/userTime/engineTime/engineTime.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/symbols/symbols.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionSet/dimensionSet.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionSet/dimensionSetIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionSet/dimensionSets.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/unitConversion.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/unitConversionIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/unitConversions.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/namedUnitConversion.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/namedUnitConversionIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionedTypes/dimensionedScalar/dimensionedScalar.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionedTypes/dimensionedSphericalTensor/dimensionedSphericalTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionedTypes/dimensionedSymmTensor/dimensionedSymmTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionedTypes/dimensionedTensor/dimensionedTensor.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/solution/solution.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/scalarMatrices/scalarMatrices.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/scalarMatrices/SVD/SVD.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/scalarMatrices/eigendecomposition/eigendecomposition.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LUscalarMatrix/LUscalarMatrix.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LUscalarMatrix/procLduMatrix.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LUscalarMatrix/procLduInterface.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrix.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrixOperations.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrixATmul.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrixUpdateMatrixInterfaces.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrixSolver.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrixSmoother.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduMatrix/lduMatrixPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/diagonalSolver/diagonalSolver.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/smoothSolver/smoothSolver.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/PCG/PCG.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/PBiCG/PBiCG.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/PBiCGStab/PBiCGStab.C
  # ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/GaussSeidel/GaussSeidelSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/symGaussSeidel/symGaussSeidelSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/nonBlockingGaussSeidel/nonBlockingGaussSeidelSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/DIC/DICSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/FDIC/FDICSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/DICGaussSeidel/DICGaussSeidelSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/DILU/DILUSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/smoothers/DILUGaussSeidel/DILUGaussSeidelSmoother.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/preconditioners/noPreconditioner/noPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/preconditioners/diagonalPreconditioner/diagonalPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/preconditioners/DICPreconditioner/DICPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/preconditioners/FDICPreconditioner/FDICPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/preconditioners/DILUPreconditioner/DILUPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/preconditioners/GAMGPreconditioner/GAMGPreconditioner.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduAddressing.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduInterface/lduInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduInterface/processorLduInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduInterface/cyclicLduInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduInterfaceFields/lduInterfaceField/lduInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduInterfaceFields/processorLduInterfaceField/processorLduInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/lduAddressing/lduInterfaceFields/cyclicLduInterfaceField/cyclicLduInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGSolver.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGSolverAgglomerateMatrix.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGSolverInterpolate.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGSolverScale.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGSolverSolve.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaces/GAMGInterface/GAMGInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaces/GAMGInterface/GAMGInterfaceNew.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaces/processorGAMGInterface/processorGAMGInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaces/processorCyclicGAMGInterface/processorCyclicGAMGInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaces/cyclicGAMGInterface/cyclicGAMGInterface.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaceFields/GAMGInterfaceField/GAMGInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaceFields/GAMGInterfaceField/GAMGInterfaceFieldNew.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaceFields/processorGAMGInterfaceField/processorGAMGInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaceFields/processorCyclicGAMGInterfaceField/processorCyclicGAMGInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/interfaceFields/cyclicGAMGInterfaceField/cyclicGAMGInterfaceField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGAgglomerations/GAMGAgglomeration/GAMGAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGAgglomerations/GAMGAgglomeration/GAMGAgglomerateLduAddressing.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGAgglomerations/pairGAMGAgglomeration/pairGAMGAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGAgglomerations/pairGAMGAgglomeration/pairGAMGAgglomerate.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGAgglomerations/algebraicPairGAMGAgglomeration/algebraicPairGAMGAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGAgglomerations/dummyAgglomeration/dummyAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGProcAgglomerations/GAMGProcAgglomeration/GAMGProcAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGProcAgglomerations/masterCoarsestGAMGProcAgglomeration/masterCoarsestGAMGProcAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGProcAgglomerations/manualGAMGProcAgglomeration/manualGAMGProcAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGProcAgglomerations/eagerGAMGProcAgglomeration/eagerGAMGProcAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGProcAgglomerations/noneGAMGProcAgglomeration/noneGAMGProcAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/lduMatrix/solvers/GAMG/GAMGProcAgglomerations/procFacesGAMGProcAgglomeration/procFacesGAMGProcAgglomeration.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/lduMesh/lduMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/lduMesh/lduPrimitiveMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LduMatrix/LduMatrix/lduMatrices.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LduMatrix/LduMatrix/solverPerformance.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LduMatrix/LduMatrix/LduInterfaceField/LduInterfaceFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LduMatrix/Smoothers/lduSmoothers.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LduMatrix/Preconditioners/lduPreconditioners.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/matrices/LduMatrix/Solvers/lduSolvers.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveShapes/line/line.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveShapes/plane/plane.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveShapes/triangle/intersection.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveShapes/objectHit/pointIndexHitList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveShapes/objectHit/pointIndexHitIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/edge/edge.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/edge/edgeIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/face/face.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/face/faceIntersection.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/face/faceContactSphere.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/face/faceAreaInContact.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/face/faceIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cell/cell.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cell/oppositeCellFace.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cell/cellIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/tetCell/tetCell.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellModeller/cellModeller.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellModel/cellModel.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellModel/cellModelIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellShape/cellShape.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellShape/cellShapeEqual.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellShape/cellShapeIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellShape/cellShapeIOList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/Identifiers/patch/patchIdentifier.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/Identifiers/patch/coupleGroupIdentifier.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshObjects/meshObjects.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/zones/pointZones/pointZone.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/zones/pointZones/pointZoneList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/zones/cellZones/cellZone.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/zones/cellZones/cellZoneList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/zones/faceZones/faceZone.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/zones/faceZones/faceZoneList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/polyPatch/polyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/polyPatch/polyPatchNew.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/basic/coupled/coupledPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/cyclic/cyclicTransform.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/cyclic/cyclicPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/cyclicSlip/cyclicSlipPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/empty/emptyPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/processorCyclic/processorCyclicPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/processor/processorPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/symmetryPlane/symmetryPlanePolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/symmetry/symmetryPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/wedge/wedgePolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/constraint/internal/internalPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/derived/wall/wallPolyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyBoundaryMesh/polyBoundaryMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyBoundaryMesh/polyBoundaryMeshEntries.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/globalMeshData/globalMeshData.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/globalMeshData/globalPoints.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/globalMeshData/globalIndex.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/globalMeshData/commSchedule.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/globalMeshData/processorTopology.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/syncTools/syncTools.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshTetDecomposition/polyMeshTetDecomposition.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshTetDecomposition/tetIndices.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshFromShapeMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshInitMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshClear.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshUpdate.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/meshObjects/Residuals/residuals.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/meshObjects/cpuLoad/cpuLoad.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshCellCells.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshCellCentresAndVols.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshCellEdges.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshCells.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshClear.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshEdgeCells.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshEdgeFaces.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshEdges.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshFaceCentresAndAreas.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshFindCell.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshPointCells.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshPointFaces.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshPointPoints.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshCellPoints.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitiveMeshCalcCellShapes.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitivePatch/patchZones.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/primitivePatch/walkPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/cellMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/hexMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/wedgeMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/prismMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/pyrMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/tetWedgeMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/tetMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/cellMatcher/degenerateMatcher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyTopoChangeMap/polyTopoChangeMap.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyTopoChangeMap/faceMapper/faceMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyTopoChangeMap/cellMapper/cellMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyTopoChangeMap/mapAddedPolyMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMeshMap/polyMeshMap.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyDistributionMap/distributionMap.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyDistributionMap/distributionMapBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyDistributionMap/polyDistributionMap.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyDistributionMap/IOdistributionMap.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/primitiveMesh/PrimitivePatch/PrimitivePatchName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointMeshMapper/pointMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointMeshMapper/pointPatchMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/pointPatch/pointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/facePointPatch/facePointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/facePointPatch/facePointPatchNew.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/basic/coupled/coupledPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/cyclic/cyclicPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/cyclicSlip/cyclicSlipPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/empty/emptyPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/processor/processorPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/processorCyclic/processorCyclicPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/symmetryPlane/symmetryPlanePointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/symmetry/symmetryPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/wedge/wedgePointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/constraint/internal/internalPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/derived/coupled/coupledFacePointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointPatches/derived/wall/wallPointPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointBoundaryMesh/pointBoundaryMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/boundBox/boundBox.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/treeBoundBox/treeBoundBox.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshTools/matchPoints.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/UniformDimensionedFields/uniformDimensionedFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/UniformGeometricFields/uniformGeometricFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/fieldMapper/fieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/forwardFieldMapper/forwardFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/forwardOrAssignFieldMapper/forwardOrAssignFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/generalFieldMapper/generalFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/identityFieldMapper/identityFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/reverseFieldMapper/reverseFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/reverseInterpolativeFieldMapper/reverseInterpolativeFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/setSizeFieldMapper/setSizeFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/fieldMappers/setSizeAndZeroFieldMapper/setSizeAndZeroFieldMapper.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/labelField/labelField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/scalarField/scalarField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/vectorField/vectorField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/vector2DField/vector2DField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/sphericalTensorField/sphericalTensorField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/diagTensorField/diagTensorField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/symmTensorField/symmTensorField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/tensorField/tensorField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/quaternionField/quaternionField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/triadField/triadField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/complexFields/complexFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/labelField/labelIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/labelField/labelFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/scalarField/scalarIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/scalarField/scalarFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/vectorField/vectorIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/vectorField/vectorFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/vector2DField/vector2DIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/vector2DField/vector2DFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/sphericalTensorField/sphericalTensorIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/sphericalTensorField/sphericalTensorFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/diagTensorField/diagTensorIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/diagTensorField/diagTensorFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/symmTensorField/symmTensorIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/symmTensorField/symmTensorFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/tensorField/tensorIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/tensorField/tensorFieldIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/quaternionField/quaternionIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/triadField/triadIOField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/Fields/transformField/transformField.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/pointPatchField/pointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/basic/calculated/calculatedPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/basic/coupled/coupledPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/basic/value/valuePointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/basic/fixedValue/fixedValuePointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/basic/zeroGradient/zeroGradientPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/cyclic/cyclicPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/cyclicSlip/cyclicSlipPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/empty/emptyPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/processor/processorPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/processorCyclic/processorCyclicPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/symmetryPlane/symmetryPlanePointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/symmetry/symmetryPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/wedge/wedgePointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/constraint/internal/internalPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/derived/slip/slipPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/derived/fixedNormalSlip/fixedNormalSlipPointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/derived/uniformFixedValue/uniformFixedValuePointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/pointPatchFields/derived/codedFixedValue/codedFixedValuePointPatchFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/fields/GeometricFields/pointFields/pointFields.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/bandCompression/bandCompression.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/preservePatchTypes/preservePatchTypes.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/interpolations/interpolationWeights/interpolationWeights/interpolationWeights.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/interpolations/interpolationWeights/linearInterpolationWeights/linearInterpolationWeights.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/interpolations/interpolationWeights/stepInterpolationWeights/stepInterpolationWeights.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/interpolations/interpolationWeights/splineInterpolationWeights/splineInterpolationWeights.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/indexedOctree/indexedOctreeName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/indexedOctree/treeDataCell.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/indexedOctree/volumeType.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/dynamicIndexedOctree/dynamicIndexedOctreeName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/dynamicIndexedOctree/dynamicTreeDataPoint.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/polygonTriangulate/polygonTriangulate.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/algorithms/solutionControl/solutionControl.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/distribution/distribution.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/distribution/distributionNew.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/unintegrable/unintegrable.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/unintegrable/unintegrableForNonZeroQ.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/exponential/exponential.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/fixedValue/fixedValue.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/multiFixedValue/multiFixedValue.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/multiNormal/multiNormal.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/normal/normal.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/RosinRammler/RosinRammler.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/standardNormal/standardNormal.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/tabulatedCumulative/tabulatedCumulative.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/tabulatedDensity/tabulatedDensity.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/distributions/uniform/uniform.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/geometry/splines/BSpline/BSpline.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/geometry/splines/CatmullRomSpline/CatmullRomSpline.C

  src/lduMatrix/lduMatrix/lduMatrix.cpp
  src/lduMatrix/lduMatrix/lduMatrixATmul.cpp
  src/lduMatrix/lduMatrix/lduMatrixOperations.cpp
  src/lduMatrix/lduMatrix/lduMatrixSmoother.cpp
  src/lduMatrix/lduMatrix/lduMatrixSolver.cpp
  src/lduMatrix/lduMatrix/lduMatrixPreconditioner.cpp
  src/lduMatrix/lduMatrix/lduMatrixUpdateMatrixInterfaces.cpp
  src/lduMatrix/smoothers/GaussSeidel/GaussSeidelSmoother.cpp
  
  src/util/result_matchers.cpp

  ${OPENFOAM_PSTREAM_SOURCES}
)

add_library(OpenFOAM::OpenFOAM ALIAS OpenFOAM)

target_link_libraries(OpenFOAM PUBLIC z)
target_compile_features(OpenFOAM PUBLIC cxx_std_20)
target_link_options(OpenFOAM PRIVATE)

target_include_directories(OpenFOAM PUBLIC
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/lnInclude
  src/lduMatrix/lduMatrix
  src/lduMatrix/smoothers/GaussSeidel
  src/util
  ${OPENFOAM_SRC_DIR}/OpenFOAM/lnInclude
  ${OPENFOAM_SRC_DIR}/OpenFOAM/include
)

target_compile_options(OpenFOAM PRIVATE
  -Wall
  -Wextra
  -Wold-style-cast
  -Wnon-virtual-dtor
  -Wno-unused-parameter
  -Wno-invalid-offsetof
  -Wno-undefined-var-template
  -Wno-unqualified-std-cast-call
  -Wno-attributes
  -ftemplate-depth-100
  -fPIC
  # -fsanitize=address
)

target_compile_definitions(OpenFOAM PRIVATE
    LIB_NAME=libOpenFOAM.so
)

target_compile_definitions(OpenFOAM PUBLIC
    linux64
    WM_SP
    WM_ARCH_OPTION=64
    WM_LABEL_SIZE=32
    NoRepository
    SPDLOG_FMT_EXTERNAL
#    TRACY_ENABLE
)

if (ENABLE_TT)
  message(STATUS "Building with Tenstorrent offloading")

  add_subdirectory(daisy-tt-rt)

  target_compile_definitions(OpenFOAM PUBLIC
    ENABLE_TT
  )
  if (VERIFY_TT)
    message(STATUS "Building with Tenstorrent verification")
    target_compile_definitions(OpenFOAM PUBLIC
      VERIFY_TT
    )
  endif()
  target_link_libraries(OpenFOAM PUBLIC
    OpenFOAM::daisy_tt_rt
  )
  
  target_sources(OpenFOAM PRIVATE
    ${TT_SOURCES}
  )
  target_include_directories(OpenFOAM PUBLIC
    src/tenstorrent
  )
endif()

if (DAISY_INSTRUMENTATION)
  message(STATUS "Building with DAISY instrumentation")
  target_compile_definitions(OpenFOAM PRIVATE
    __DAISY_INSTRUMENTATION
  )
  target_link_libraries(OpenFOAM PRIVATE
    daisy_rtl arg_capture_io
  )
endif()

add_library(finiteVolume SHARED
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshGeometry.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMesh.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/singleCellFvMesh/singleCellFvMesh.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/zeroDimensionalFvMesh/zeroDimensionalFvMesh.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvCellSet/fvCellSet.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvBoundaryMesh/fvBoundaryMesh.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/fvPatch/fvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/fvPatch/fvPatchNew.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/basic/coupled/coupledFvPatch.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/cyclic/cyclicFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/cyclicSlip/cyclicSlipFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/empty/emptyFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/processor/processorFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/processorCyclic/processorCyclicFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/symmetryPlane/symmetryPlaneFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/symmetry/symmetryFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/wedge/wedgeFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/internal/internalFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/nonConformal/nonConformalFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/nonConformalCoupled/nonConformalCoupledFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/nonConformalCyclic/nonConformalCyclicFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/nonConformalError/nonConformalErrorFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/constraint/nonConformalProcessorCyclic/nonConformalProcessorCyclicFvPatch.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/wall/wallFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/mapped/mappedFvPatchBaseBase.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/mapped/mappedFvPatchBase.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/mapped/mappedFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/mapped/mappedWallFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/mapped/mappedExtrudedWallFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/mapped/mappedInternalFvPatch.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/nonConformalMapped/nonConformalMappedFvPatchBase.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvPatches/derived/nonConformalMapped/nonConformalMappedWallFvPatch.C


  ${OPENFOAM_SRC_DIR}/finiteVolume/algorithms/FvFaceCellWave/FvFaceCellWaveName.C


  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/fvPatchDistWave/fvPatchDistWave.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/nearWallDist/nearWallDist.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/wallDist/wallDist.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/patchDistMethods/patchDistMethod/patchDistMethod.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/patchDistMethods/meshWave/meshWavePatchDistMethod.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/patchDistMethods/Poisson/PoissonPatchDistMethod.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/wallDist/patchDistMethods/advectionDiffusion/advectionDiffusionPatchDistMethod.C


  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshMapper/fvPatchMapper.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshMapper/fvSurfaceMapper.C


  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/extendedCellToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/extendedCentredCellToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/globalIndexStencils/cellToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/globalIndexStencils/CFCCellToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/globalIndexStencils/CPCCellToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/globalIndexStencils/CECCellToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/MeshObjects/centredCECCellToCellStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/MeshObjects/centredCFCCellToCellStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToCell/MeshObjects/centredCPCCellToCellStencilObject.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/globalIndexStencils/cellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/globalIndexStencils/CFCCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/globalIndexStencils/CECCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/globalIndexStencils/CPCCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/globalIndexStencils/FECCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/extendedCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/extendedCentredCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/extendedUpwindCellToFaceStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/centredCECCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/centredCFCCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/centredCPCCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/centredFECCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/upwindCECCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/upwindCFCCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/upwindCPCCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/upwindFECCellToFaceStencilObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/cellToFace/MeshObjects/pureUpwindCFCCellToFaceStencilObject.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/faceToCell/globalIndexStencils/faceToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/faceToCell/globalIndexStencils/CFCFaceToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/faceToCell/extendedFaceToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/faceToCell/extendedCentredFaceToCellStencil.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/extendedStencil/faceToCell/MeshObjects/centredCFCFaceToCellStencilObject.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshTopoChangers/fvMeshTopoChanger/fvMeshTopoChanger.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshTopoChangers/fvMeshTopoChanger/fvMeshTopoChangerNew.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshTopoChangers/none/none_fvMeshTopoChanger.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshTopoChangers/list/list_fvMeshTopoChanger.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshDistributors/fvMeshDistributor/fvMeshDistributor.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshDistributors/fvMeshDistributor/fvMeshDistributorNew.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshDistributors/none/none_fvMeshDistributor.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshMovers/fvMeshMover/fvMeshMover.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshMovers/fvMeshMover/fvMeshMoverNew.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshMovers/none/none_fvMeshMover.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshStitchers/fvMeshStitcher/fvMeshStitcher.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshStitchers/fvMeshStitcher/fvMeshStitcherNew.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshStitchers/fvMeshStitcher/fvMeshStitcherTools.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshStitchers/fvMeshStitcher/conformedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshStitchers/fvMeshStitcher/conformedFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMesh/fvMeshStitchers/stationary/stationary_fvMeshStitcher.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/functionObjects/fvMeshFunctionObject/fvMeshFunctionObject.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/functionObjects/fvModelFunctionObject/fvModelFunctionObject.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/fvPatchField/fvPatchFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/basicSymmetry/basicSymmetryFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/calculated/calculatedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/extrapolatedCalculated/extrapolatedCalculatedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/coupled/coupledFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/directionMixed/directionMixedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/fixedGradient/fixedGradientFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/fixedValue/fixedValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/mixed/mixedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/sliced/slicedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/transform/transformFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/transform/transformFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/basic/zeroGradient/zeroGradientFvPatchFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/cyclic/cyclicFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/cyclicSlip/cyclicSlipFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/empty/emptyFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/jumpCyclic/jumpCyclicFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/processor/processorFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/processor/processorFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/processorCyclic/processorCyclicFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/symmetryPlane/symmetryPlaneFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/symmetryPlane/symmetryPlaneFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/symmetry/symmetryFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/wedge/wedgeFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/wedge/wedgeFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/internal/internalFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/nonConformalCyclic/nonConformalCyclicFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/nonConformalError/nonConformalErrorFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/constraint/nonConformalProcessorCyclic/nonConformalProcessorCyclicFvPatchFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/advective/advectiveFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/codedFixedValue/codedFixedValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/codedMixed/codedMixedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/externalCoupledMixed/externalCoupledMixedFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fanPressureJump/fanPressureJumpFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fanPressure/fanPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedFluxPressure/fixedFluxPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedFluxExtrapolatedPressure/fixedFluxExtrapolatedPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedInternalValue/fixedInternalValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedJump/fixedJumpFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedMean/fixedMeanFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedNormalSlip/fixedNormalSlipFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedPressureCompressibleDensity/fixedPressureCompressibleDensityFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/flowRateInletVelocity/flowRateInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/flowRateInletVelocity/laminarBL/laminarBL.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/flowRateInletVelocity/turbulentBL/turbulentBL.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/flowRateOutletVelocity/flowRateOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fluxCorrectedVelocity/fluxCorrectedVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/freestream/freestreamFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/freestreamVelocity/freestreamVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/freestreamPressure/freestreamPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/inletOutlet/inletOutletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/zeroInletOutlet/zeroInletOutletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/inletOutletTotalTemperature/inletOutletTotalTemperatureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/mappedFlowRateVelocity/mappedFlowRateVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/mappedInternalValue/mappedInternalValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/mappedValue/mappedValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/mappedVelocityFlux/mappedVelocityFluxFvPatchField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/matchedFlowRateOutletVelocity/matchedFlowRateOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/noSlip/noSlipFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/movingWallVelocity/movingWallVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/movingWallSlipVelocity/movingWallSlipVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/movingMappedWallVelocity/movingMappedWallVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/outletInlet/outletInletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/outletMappedUniformInlet/outletMappedUniformInletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedMeanOutletInlet/fixedMeanOutletInletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/partialSlip/partialSlipFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/phaseHydrostaticPressure/phaseHydrostaticPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureDirectedInletOutletVelocity/pressureDirectedInletOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureDirectedInletVelocity/pressureDirectedInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureInletOutletParSlipVelocity/pressureInletOutletParSlipVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureInletOutletVelocity/pressureInletOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureInletUniformVelocity/pressureInletUniformVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureInletVelocity/pressureInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressureNormalInletOutletVelocity/pressureNormalInletOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedNormalInletOutletVelocity/fixedNormalInletOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/rotatingPressureInletOutletVelocity/rotatingPressureInletOutletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/rotatingTotalPressure/rotatingTotalPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/rotatingWallVelocity/rotatingWallVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/slip/slipFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/supersonicFreestream/supersonicFreestreamFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/surfaceNormalFixedValue/surfaceNormalFixedValueFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/surfaceNormalUniformFixedValue/surfaceNormalUniformFixedValueFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/swirlFlowRateInletVelocity/swirlFlowRateInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/swirlInletVelocity/swirlInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/syringePressure/syringePressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/timeVaryingMappedFixedValue/timeVaryingMappedFvPatchFieldName.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/timeVaryingMappedFixedValue/timeVaryingMappedFixedValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/dynamicPressure/dynamicPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/totalPressure/totalPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/entrainmentPressure/entrainmentPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/totalTemperature/totalTemperatureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/translatingWallVelocity/translatingWallVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/turbulentInlet/turbulentInletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/turbulentIntensityKineticEnergyInlet/turbulentIntensityKineticEnergyInletFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/uniformDensityHydrostaticPressure/uniformDensityHydrostaticPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/uniformFixedGradient/uniformFixedGradientFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/uniformFixedValue/uniformFixedValueFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/uniformInletOutlet/uniformInletOutletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/uniformJump/uniformJumpFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/uniformTotalPressure/uniformTotalPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/variableHeightFlowRate/variableHeightFlowRateFvPatchField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/variableHeightFlowRateInletVelocity/variableHeightFlowRateInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/outletPhaseMeanVelocity/outletPhaseMeanVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/waveTransmissive/waveTransmissiveFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/waveSurfacePressure/waveSurfacePressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/interstitialInletVelocity/interstitialInletVelocityFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedProfile/fixedProfileFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/plenumPressure/plenumPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/interfaceCompression/interfaceCompressionFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/pressure/pressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/PrghPressure/prghPressureFvPatchScalarFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/prghTotalHydrostaticPressure/prghTotalHydrostaticPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/fixedValueInletOutlet/fixedValueInletOutletFvPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/transonicEntrainmentPressure/transonicEntrainmentPressureFvPatchScalarField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvPatchFields/derived/prghCyclicPressure/prghCyclicPressureFvPatchScalarField.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/fvsPatchField/fvsPatchFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/basic/calculated/calculatedFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/basic/coupled/coupledFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/basic/fixedValue/fixedValueFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/basic/sliced/slicedFvsPatchFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/cyclic/cyclicFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/cyclicSlip/cyclicSlipFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/empty/emptyFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/processor/processorFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/processorCyclic/processorCyclicFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/symmetryPlane/symmetryPlaneFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/symmetry/symmetryFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/wedge/wedgeFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/internal/internalFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/nonConformalCyclic/nonConformalCyclicFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/nonConformalError/nonConformalErrorFvsPatchFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/constraint/nonConformalProcessorCyclic/nonConformalProcessorCyclicFvsPatchFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/derived/polyFaces/polyFacesFvsPatchLabelField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/derived/nonConformalPolyFaces/nonConformalPolyFacesFvsPatchLabelField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvsPatchFields/derived/nonConformalMappedPolyFaces/nonConformalMappedPolyFacesFvsPatchLabelField.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvFieldSources/fvFieldSource/fvFieldSources.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvFieldSources/derived/internal/internalFvFieldSources.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvFieldSources/derived/uniformFixedValue/uniformFixedValueFvFieldSources.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvFieldSources/derived/uniformInletOutlet/uniformInletOutletFvFieldSources.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/fvFieldSources/derived/turbulentIntensityKineticEnergy/turbulentIntensityKineticEnergyFvScalarFieldSource.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/volFields/volFields.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fields/surfaceFields/surfaceFields.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMatrices/fvMatrices.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMatrices/fvScalarMatrix/fvScalarMatrix.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMatrices/solvers/MULES/MULES.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMatrices/solvers/GAMGSymSolver/GAMGAgglomerations/faceAreaPairGAMGAgglomeration/faceAreaPairGAMGAgglomeration.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolation/interpolations.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationCell/makeInterpolationCell.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationCellPatchConstrained/makeInterpolationCellPatchConstrained.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationVolPointInterpolation/makeInterpolationVolPointInterpolation.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationCellPoint/cellPointWeight/cellPointWeight.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationCellPoint/makeInterpolationCellPoint.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationCellPointFace/makeInterpolationCellPointFace.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationCellPointWallModified/makeInterpolationCellPointWallModified.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationPointMVC/pointMVCWeight.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/interpolation/interpolationPointMVC/makeInterpolationPointMVC.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/volPointInterpolation/volPointInterpolation.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/volPointInterpolation/pointConstraints.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/surfaceInterpolation/surfaceInterpolation.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/surfaceInterpolationScheme/surfaceInterpolationSchemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/blendedSchemeBase/blendedSchemeBaseName.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/linear/linear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/pointLinear/pointLinear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/midPoint/midPoint.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/downwind/downwind.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/weighted/weighted.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/cubic/cubic.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/skewCorrected/skewCorrectionVectors.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/skewCorrected/skewCorrected.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/outletStabilised/outletStabilised.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/phaseStabilised/phaseStabilised.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/reverseLinear/reverseLinear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/clippedLinear/clippedLinear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/harmonic/harmonic.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/fixedBlended/fixedBlended.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/localBlended/localBlended.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/limiterBlended/limiterBlended.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/CoBlended/CoBlended.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/cellCoBlended/cellCoBlended.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/deferred/deferred.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/localMax/localMax.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/localMin/localMin.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/linearFit/linearFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/biLinearFit/biLinearFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/quadraticLinearFit/quadraticLinearFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/quadraticFit/quadraticFit.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/quadraticLinearUpwindFit/quadraticLinearUpwindFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/quadraticUpwindFit/quadraticUpwindFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/cubicUpwindFit/cubicUpwindFit.C
#   ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/quadraticLinearPureUpwindFit/quadraticLinearPureUpwindFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/linearPureUpwindFit/linearPureUpwindFit.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/linearUpwind/linearUpwind.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/linearUpwind/linearUpwindV.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/schemes/LUST/LUST.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/limitedSurfaceInterpolationScheme/limitedSurfaceInterpolationSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/upwind/upwind.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/blended/blended.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/Gamma/Gamma.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/SFCD/SFCD.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/Minmod/Minmod.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/vanLeer/vanLeer.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/vanAlbada/vanAlbada.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/OSPRE/OSPRE.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/limitedLinear/limitedLinear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/limitedCubic/limitedCubic.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/SuperBee/SuperBee.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/QUICK/QUICK.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/MUSCL/MUSCL.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/UMIST/UMIST.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/Phi/Phi.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/filteredLinear/filteredLinear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/filteredLinear2/filteredLinear2.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/filteredLinear3/filteredLinear3.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/limitedSchemes/limitWith/limitWith.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/multivariateSurfaceInterpolationScheme/multivariateSurfaceInterpolationSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/multivariateSelectionScheme/multivariateSelectionSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/multivariateIndependentScheme/multivariateIndependentSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/upwind/multivariateUpwind.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/Gamma/multivariateGamma.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/vanLeer/multivariateVanLeer.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/Minmod/multivariateMinmod.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/SuperBee/multivariateSuperBee.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/MUSCL/multivariateMUSCL.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/limitedLinear/multivariateLimitedLinear.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/interpolation/surfaceInterpolation/multivariateSchemes/limitedCubic/multivariateLimitedCubic.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/fv/fv.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/fvSchemes/fvSchemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/ddtScheme/ddtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/steadyStateDdtScheme/steadyStateDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/EulerDdtScheme/EulerDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/CoEulerDdtScheme/CoEulerDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/SLTSDdtScheme/SLTSDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/localEulerDdtScheme/localEulerDdt.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/localEulerDdtScheme/localEulerDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/backwardDdtScheme/backwardDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/CrankNicolsonDdtScheme/CrankNicolsonDdtSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/ddtSchemes/boundedDdtScheme/boundedDdtSchemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/d2dt2Schemes/d2dt2Scheme/d2dt2Schemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/d2dt2Schemes/steadyStateD2dt2Scheme/steadyStateD2dt2Schemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/d2dt2Schemes/EulerD2dt2Scheme/EulerD2dt2Schemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/divSchemes/divScheme/divSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/divSchemes/gaussDivScheme/gaussDivSchemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/gradScheme/gradSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/gaussGrad/gaussGrads.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/leastSquaresGrad/leastSquaresVectors.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/leastSquaresGrad/leastSquaresGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/LeastSquaresGrad/LeastSquaresGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/fourthGrad/fourthGrads.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/limitedGradSchemes/faceLimitedGrad/faceLimitedGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/limitedGradSchemes/cellLimitedGrad/cellLimitedGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/limitedGradSchemes/faceMDLimitedGrad/faceMDLimitedGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/gradSchemes/limitedGradSchemes/cellMDLimitedGrad/cellMDLimitedGrads.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/snGradScheme/snGradSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/correctedSnGrad/correctedSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/faceCorrectedSnGrad/faceCorrectedSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/limitedSnGrad/limitedSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/uncorrectedSnGrad/uncorrectedSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/orthogonalSnGrad/orthogonalSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/quadraticFitSnGrad/quadraticFitSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/linearFitSnGrad/linearFitSnGrads.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/snGradSchemes/phaseStabilisedSnGrad/phaseStabilisedSnGrads.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/convectionSchemes/convectionScheme/convectionSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/convectionSchemes/gaussConvectionScheme/gaussConvectionSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/convectionSchemes/multivariateGaussConvectionScheme/multivariateGaussConvectionSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/convectionSchemes/boundedConvectionScheme/boundedConvectionSchemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/laplacianSchemes/laplacianScheme/laplacianSchemes.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/laplacianSchemes/gaussLaplacianScheme/gaussLaplacianSchemes.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/fvc/fvcMeshPhi.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/fvc/fvcSmooth/fvcSmooth.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/finiteVolume/fvc/fvcReconstructMag.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/findRefCell/findRefCell.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/constrainHbyA/constrainHbyA.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/adjustPhi/adjustPhi.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/bound/bound.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/correctPhi/fvCorrectPhi.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/correctPhi/correctUphiBCs.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/pressureReference/pressureReference.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/levelSet/levelSet.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/surfaceToVolVelocity/surfaceToVolVelocity.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/buoyancy/buoyancy.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/solutionControl/singleRegionSolutionControl/singleRegionSolutionControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/solutionControl/multiRegionSolutionControl/multiRegionSolutionControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/solutionControl/nonOrthogonalSolutionControl/nonOrthogonalSolutionControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/solutionControl/fluidSolutionControl/fluidSolutionControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/simpleControl/simpleControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/pisoControl/pisoControl.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/pimpleControl/pimpleControl/pimpleControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/pimpleControl/pimpleNoLoopControl/pimpleNoLoopControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/pimpleControl/pimpleLoop/pimpleLoop.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/pimpleControl/pimpleSingleRegionControl/pimpleSingleRegionControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/pimpleControl/pimpleMultiRegionControl/pimpleMultiRegionControl.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/convergenceControl/convergenceControl/convergenceControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/convergenceControl/singleRegionConvergenceControl/singleRegionConvergenceControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/convergenceControl/correctorConvergenceControl/correctorConvergenceControl.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/solutionControl/convergenceControl/singleRegionCorrectorConvergenceControl/singleRegionCorrectorConvergenceControl.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/porosityModel/porosityModel.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/porosityModel/porosityModelNew.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/porosityModel/porosityModelList.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/porosityModel/IOporosityModelList.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/DarcyForchheimer/DarcyForchheimer.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/fixedCoeff/fixedCoeff.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/powerLaw/powerLaw.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/porosityModel/solidification/solidification.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/MRFZone.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/MRFZoneList.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/IOMRFZoneList.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/derivedFvPatchFields/MRFPatchField/MRFPatchField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/derivedFvPatchFields/MRFnoSlip/MRFnoSlipFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/derivedFvPatchFields/MRFslip/MRFslipFvPatchVectorField.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/MRF/derivedFvPatchFields/MRFFreestreamVelocity/MRFFreestreamVelocityFvPatchVectorField.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvModels/fvModel.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvModels/fvModels.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvModels/includeFvModelEntry/includeFvModelEntry.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvConstraints/fvConstraint.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvConstraints/fvConstraints.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvConstraints/includeFvConstraintEntry/includeFvConstraintEntry.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvSource/fvSource.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvSource/fvSpecificSource.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/cfdTools/general/fvSource/fvTotalSource.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/sets/cellSources/fieldToCell/fieldToCell.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/sets/faceSources/patchFluxToFace/patchFluxToFace.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/solver/solver.C
  ${OPENFOAM_SRC_DIR}/finiteVolume/solver/solverNew.C

  ${OPENFOAM_SRC_DIR}/finiteVolume/fvMeshToFvMesh/fvMeshToFvMesh.C
)


add_library(OpenFOAM::finiteVolume ALIAS finiteVolume)

target_link_libraries(finiteVolume PUBLIC OpenFOAM::OpenFOAM triSurface meshTools)
target_compile_features(finiteVolume PUBLIC cxx_std_20)
target_link_directories(finiteVolume PUBLIC ${OPENFOAM_LIB_DIR})
target_link_options(finiteVolume PRIVATE)

target_include_directories(finiteVolume PUBLIC
  ${OPENFOAM_SRC_DIR}/finiteVolume/lnInclude
  ${OPENFOAM_SRC_DIR}/triSurface/lnInclude
  ${OPENFOAM_SRC_DIR}/meshTools/lnInclude
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/lnInclude
  ${OPENFOAM_SRC_DIR}/OpenFOAM/include
  ${OPENFOAM_SRC_DIR}/OpenFOAM/lnInclude
)

target_compile_options(finiteVolume PRIVATE
  -Wall
  -Wextra
  -Wold-style-cast
  -Wnon-virtual-dtor
  -Wno-unused-parameter
  -Wno-invalid-offsetof
  -Wno-undefined-var-template
  -Wno-unqualified-std-cast-call
  -Wno-attributes
  -ftemplate-depth-100
  -fPIC
  # -fsanitize=address
)

target_compile_definitions(finiteVolume PRIVATE
    LIB_NAME=libfiniteVolume.so
)

target_compile_definitions(finiteVolume PUBLIC
    linux64
    WM_SP
    WM_ARCH_OPTION=64
    WM_LABEL_SIZE=32
    NoRepository
    SPDLOG_FMT_EXTERNAL
)


add_subdirectory(tests)

if(ENABLE_TT)
  add_subdirectory(tt-bench-harness)
endif()

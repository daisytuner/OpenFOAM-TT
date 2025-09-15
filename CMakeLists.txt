cmake_minimum_required(VERSION 3.26)

project(OpenFOAM-TT VERSION "1.0.0" LANGUAGES CXX)

set(OPENFOAM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/OpenFOAM-dev/src)

add_library(OpenFOAM SHARED
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
  ${OPENFOAM_SRC_DIR}/Pstream/dummy/UPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/HashTables/HashTable/HashTableCore.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/LinkedLists/linkTypes/DLListBase/DLListBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/containers/LinkedLists/linkTypes/SLListBase/SLListBase.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryEntry/dictionaryEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/entry/entry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryListEntry/dictionaryListEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/entry/entryIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/keyType/keyType.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/functionEntry/functionEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/functionObjectList/functionObjectList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/primitiveEntry/primitiveEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/dlLibraryTable/dlLibraryTable.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/error/error.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/error/IOerror.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/objectRegistry/objectRegistry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobject/IOobject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/IOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOobjects/IOdictionary/timeIOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/controlIOdictionary/controlIOdictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams/Istream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams/IOstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/Pstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/IPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/UIPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/UOPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/gzstream/gzstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams/Ostream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Fstreams/IFstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Fstreams/OFstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/ISstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/OSstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Tstreams/ITstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/token/token.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/token/tokenIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/regIOobject/regIOobject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/findTimes.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/Time.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/TimePaths.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/TimeState.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/instant/instant.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/Time/userTime/userTime/userTime.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/argList/argList.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/clock/clock.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/debug/debug.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/global/jobInfo/jobInfo.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionSet/dimensionSet.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionSet/dimensionSets.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/dimensionSet/dimensionSetIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/pointMesh/pointMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyMesh.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/polyMesh/polyPatches/polyPatch/polyPatch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/meshes/meshShapes/face/face.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/bools/Switch/Switch.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/hashes/Hasher/Hasher.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/hashes/SHA1/SHA1.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/hashes/SHA1/SHA1Digest.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/nullObject/nullObject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/fileName/fileName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/functionName/functionName.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/word/word.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/string/string.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/stringOps/stringOps.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/variable/variable.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/primitives/strings/verbatimString/verbatimString.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/symbols/symbols.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/unitConversion.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/unitConversions.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/namedUnitConversionIO.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/inputModeEntry/inputModeEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/calcIncludeEntry/calcIncludeEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/functionEntries/includeFuncEntry/includeFuncEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dictionary/dictionaryListEntry/dictionaryListEntry.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/functionObject/functionObject.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/functionObjects/functionObjectList/wordAndDictionary.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/dynamicLibrary/dlLibraryTable/dlLibraryTable.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Pstreams/OPstream.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/unitConversion/namedUnitConversion.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/IOstreams.C
  ${OPENFOAM_SRC_DIR}/OpenFOAM/db/IOstreams/Sstreams/prefixOSstream.C
)
target_link_libraries(OpenFOAM PUBLIC z)
target_compile_features(OpenFOAM PUBLIC cxx_std_17)

target_include_directories(OpenFOAM PUBLIC
  ${OPENFOAM_SRC_DIR}/OSspecific/POSIX/lnInclude
  ${OPENFOAM_SRC_DIR}/OpenFOAM/lnInclude
  ${OPENFOAM_SRC_DIR}/OpenFOAM/include
)

target_precompile_headers(OpenFOAM PRIVATE "${OPENFOAM_SRC_DIR}/OpenFOAM/include/OSspecific.H")

target_compile_definitions(OpenFOAM PUBLIC
    WM_PRECISION_OPTION=SP
    WM_LABEL_SIZE=32
    WM_SP
    NoRepository
)

add_subdirectory(tests)

#pragma once

#include <Field.H>
#include <scalarField.H>
#include <FieldField.H>
#include <Ostream.H>
#include <lduInterfaceFieldPtrsList.H>
#include <ttLduData.hpp>

namespace tt::daisy::foam {

Foam::Ostream& operator<<(Foam::Ostream& os, const tt_ldu_meta& tt_meta);

extern std::unordered_map<const void*, tt_ldu_meta> ldu_tt_meta_map;

void verify_interfaces_noop(const Foam::lduInterfaceFieldPtrsList& interfaces);

tt_ldu_meta& ensure_lduMat_on_device(class KernelLauncher& k, const class Foam::lduMatrix* lduMat, bool reserve_all_parts = false);

void clear_tt_meta(const void* key, bool clear_addrs, bool clear_contents);

}  // namespace tt::daisy::foam
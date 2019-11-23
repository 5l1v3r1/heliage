#include "sm83.h"
#include "logging.h"

SM83::SM83(MMU& mmu)
    : mmu(mmu) {
    af = 0;
    bc = 0;
    de = 0;
    hl = 0;
    sp = 0;
    pc = 0;
    pc_at_opcode = 0;
    interrupts_enabled = false;
}

void SM83::Tick() {
    HandleInterrupts();

    pc_at_opcode = pc;
    u8 opcode = GetByteFromPC();
    // LTRACE("executing opcode 0x%02X at 0x%04X", opcode, pc_at_opcode);
    if (!ExecuteOpcode(opcode, pc_at_opcode)) {
        std::exit(0);
    }
}

u8 SM83::GetByteFromPC() {
    u8 byte = mmu.Read8(pc++);
    return byte;
}

u16 SM83::GetWordFromPC() {
    u8 low = GetByteFromPC();
    u8 high = GetByteFromPC();

    u16 word = (high << 8) | low;
    return word;
}

void SM83::SetZeroFlag(bool b) {
    if (b) {
        f |= static_cast<u8>(Flags::Zero);
    } else {
        f &= ~static_cast<u8>(Flags::Zero);
    }
}

void SM83::SetNegateFlag(bool b) {
    if (b) {
        f |= static_cast<u8>(Flags::Negate);
    } else {
        f &= ~static_cast<u8>(Flags::Negate);
    }
}

void SM83::SetHalfCarryFlag(bool b) {
    if (b) {
        f |= static_cast<u8>(Flags::HalfCarry);
    } else {
        f &= ~static_cast<u8>(Flags::HalfCarry);
    }
}

void SM83::SetCarryFlag(bool b) {
    if (b) {
        f |= static_cast<u8>(Flags::Carry);
    } else {
        f &= ~static_cast<u8>(Flags::Carry);
    }
}

bool SM83::HasFlag(Flags flag) {
    return f & static_cast<u8>(flag);
}

void SM83::StackPush(u16* word_reg) {
    sp--;
    mmu.Write8(sp, static_cast<u8>((*word_reg >> 8) & 0xFF));
    sp--;
    mmu.Write8(sp, static_cast<u8>(*word_reg & 0xFF));
}

void SM83::StackPop(u16* word_reg) {
    u8 low = mmu.Read8(sp);
    sp++;
    u8 high = mmu.Read8(sp);
    sp++;

    u16 value = (high << 8) | low;
    *word_reg = value; 
}

bool SM83::ExecuteOpcode(const u8 opcode, u16 pc_at_opcode)
{
    switch (opcode) {
        case 0xCB:
            if (!ExecuteCBOpcode(GetByteFromPC(), pc_at_opcode)) {
                return false;
            }
            break;

#define INSTR(opcode, instr, ...) case opcode: instr; break
        INSTR(0x00, nop());
        INSTR(0x01, ld_bc_d16());
        INSTR(0x02, ld_dbc_a());
        INSTR(0x03, inc_bc());
        INSTR(0x04, inc_b());
        INSTR(0x05, dec_b());
        INSTR(0x06, ld_b_d8());
        INSTR(0x07, rlca());
        INSTR(0x08, ld_da16_sp());
        INSTR(0x09, add_hl_bc());
        INSTR(0x0A, ld_a_dbc());
        INSTR(0x0B, dec_bc());
        INSTR(0x0C, inc_c());
        INSTR(0x0D, dec_c());
        INSTR(0x0E, ld_c_d8());
        INSTR(0x0F, rrca());
        INSTR(0x11, ld_de_d16());
        INSTR(0x12, ld_dde_a());
        INSTR(0x13, inc_de());
        INSTR(0x14, inc_d());
        INSTR(0x15, dec_d());
        INSTR(0x16, ld_d_d8());
        INSTR(0x17, rla());
        INSTR(0x18, jr_r8());
        INSTR(0x19, add_hl_de());
        INSTR(0x1A, ld_a_dde());
        INSTR(0x1B, dec_de());
        INSTR(0x1C, inc_e());
        INSTR(0x1D, dec_e());
        INSTR(0x1E, ld_e_d8());
        INSTR(0x1F, rra());
        INSTR(0x20, jr_nz_r8());
        INSTR(0x21, ld_hl_d16());
        INSTR(0x22, ld_dhli_a());
        INSTR(0x23, inc_hl());
        INSTR(0x24, inc_h());
        INSTR(0x25, dec_h());
        INSTR(0x26, ld_h_d8());
        INSTR(0x27, daa());
        INSTR(0x28, jr_z_r8());
        INSTR(0x29, add_hl_hl());
        INSTR(0x2A, ld_a_dhli());
        INSTR(0x2B, dec_hl());
        INSTR(0x2C, inc_l());
        INSTR(0x2D, dec_l());
        INSTR(0x2E, ld_l_d8());
        INSTR(0x2F, cpl());
        INSTR(0x30, jr_nc_r8());
        INSTR(0x31, ld_sp_d16());
        INSTR(0x32, ld_dhld_a());
        INSTR(0x33, inc_sp());
        INSTR(0x34, inc_dhl());
        INSTR(0x35, dec_dhl());
        INSTR(0x36, ld_dhl_d8());
        INSTR(0x37, scf());
        INSTR(0x38, jr_c_r8());
        INSTR(0x3A, ld_a_dhld());
        INSTR(0x3C, inc_a());
        INSTR(0x3D, dec_a());
        INSTR(0x3E, ld_a_d8());
        INSTR(0x3F, ccf());
        INSTR(0x40, ld_b_b());
        INSTR(0x41, ld_b_c());
        INSTR(0x42, ld_b_d());
        INSTR(0x43, ld_b_e());
        INSTR(0x44, ld_b_h());
        INSTR(0x45, ld_b_l());
        INSTR(0x46, ld_b_dhl());
        INSTR(0x47, ld_b_a());
        INSTR(0x48, ld_c_b());
        INSTR(0x49, ld_c_c());
        INSTR(0x4A, ld_c_d());
        INSTR(0x4B, ld_c_e());
        INSTR(0x4C, ld_c_h());
        INSTR(0x4D, ld_c_l());
        INSTR(0x4E, ld_c_dhl());
        INSTR(0x4F, ld_c_a());
        INSTR(0x50, ld_d_b());
        INSTR(0x51, ld_d_c());
        INSTR(0x52, ld_d_d());
        INSTR(0x53, ld_d_e());
        INSTR(0x54, ld_d_h());
        INSTR(0x55, ld_d_l());
        INSTR(0x56, ld_d_dhl());
        INSTR(0x57, ld_d_a());
        INSTR(0x58, ld_e_b());
        INSTR(0x59, ld_e_c());
        INSTR(0x5A, ld_e_d());
        INSTR(0x5B, ld_e_e());
        INSTR(0x5C, ld_e_h());
        INSTR(0x5D, ld_e_l());
        INSTR(0x5E, ld_e_dhl());
        INSTR(0x5F, ld_e_a());
        INSTR(0x60, ld_h_b());
        INSTR(0x61, ld_h_c());
        INSTR(0x62, ld_h_d());
        INSTR(0x63, ld_h_e());
        INSTR(0x64, ld_h_h());
        INSTR(0x65, ld_h_l());
        INSTR(0x66, ld_h_dhl());
        INSTR(0x67, ld_h_a());
        INSTR(0x68, ld_l_b());
        INSTR(0x69, ld_l_c());
        INSTR(0x6A, ld_l_d());
        INSTR(0x6B, ld_l_e());
        INSTR(0x6C, ld_l_h());
        INSTR(0x6D, ld_l_l());
        INSTR(0x6E, ld_l_dhl());
        INSTR(0x6F, ld_l_a());
        INSTR(0x70, ld_dhl_b());
        INSTR(0x71, ld_dhl_c());
        INSTR(0x72, ld_dhl_d());
        INSTR(0x73, ld_dhl_e());
        INSTR(0x74, ld_dhl_h());
        INSTR(0x75, ld_dhl_l());
        INSTR(0x77, ld_dhl_a());
        INSTR(0x78, ld_a_b());
        INSTR(0x79, ld_a_c());
        INSTR(0x7A, ld_a_d());
        INSTR(0x7B, ld_a_e());
        INSTR(0x7C, ld_a_h());
        INSTR(0x7D, ld_a_l());
        INSTR(0x7E, ld_a_dhl());
        INSTR(0x7F, ld_a_a());
        INSTR(0x80, LTRACE("ADD A, B"); add_a_r(b));
        INSTR(0x81, LTRACE("ADD A, C"); add_a_r(c));
        INSTR(0x82, LTRACE("ADD A, D"); add_a_r(d));
        INSTR(0x83, LTRACE("ADD A, E"); add_a_r(e));
        INSTR(0x84, LTRACE("ADD A, H"); add_a_r(h));
        INSTR(0x85, LTRACE("ADD A, L"); add_a_r(l));
        INSTR(0x86, add_a_dhl());
        INSTR(0x87, LTRACE("ADD A, A"); add_a_r(a));
        INSTR(0x88, LTRACE("ADC A, B"); adc_a_r(b));
        INSTR(0x89, LTRACE("ADC A, C"); adc_a_r(c));
        INSTR(0x8A, LTRACE("ADC A, D"); adc_a_r(d));
        INSTR(0x8B, LTRACE("ADC A, E"); adc_a_r(e));
        INSTR(0x8C, LTRACE("ADC A, H"); adc_a_r(h));
        INSTR(0x8D, LTRACE("ADC A, L"); adc_a_r(l));
        INSTR(0x8E, LTRACE("ADC A, (HL)"); adc_a_r(mmu.Read8(hl)));
        INSTR(0x8F, LTRACE("ADC A, A"); adc_a_r(a));
        INSTR(0x90, LTRACE("SUB B"); sub_r(b));
        INSTR(0x91, LTRACE("SUB C"); sub_r(c));
        INSTR(0x92, LTRACE("SUB D"); sub_r(d));
        INSTR(0x93, LTRACE("SUB E"); sub_r(e));
        INSTR(0x94, LTRACE("SUB H"); sub_r(h));
        INSTR(0x95, LTRACE("SUB L"); sub_r(l));
        INSTR(0x96, LTRACE("SUB (HL)"); sub_r(mmu.Read8(hl)));
        INSTR(0x97, LTRACE("SUB A"); sub_r(a));
        INSTR(0x98, LTRACE("SBC B"); sbc_r(b));
        INSTR(0x99, LTRACE("SBC C"); sbc_r(c));
        INSTR(0x9A, LTRACE("SBC D"); sbc_r(d));
        INSTR(0x9B, LTRACE("SBC E"); sbc_r(e));
        INSTR(0x9C, LTRACE("SBC H"); sbc_r(h));
        INSTR(0x9D, LTRACE("SBC L"); sbc_r(l));
        INSTR(0x9E, LTRACE("SBC (HL)"); sbc_r(mmu.Read8(hl)));
        INSTR(0x9F, LTRACE("SBC A"); sbc_r(a));
        INSTR(0xA0, LTRACE("AND B"); and_r(b));
        INSTR(0xA1, LTRACE("AND C"); and_r(c));
        INSTR(0xA2, LTRACE("AND D"); and_r(d));
        INSTR(0xA3, LTRACE("AND E"); and_r(e));
        INSTR(0xA4, LTRACE("AND H"); and_r(h));
        INSTR(0xA5, LTRACE("AND L"); and_r(l));
        INSTR(0xA6, LTRACE("AND (HL)"); and_r(mmu.Read8(hl)));
        INSTR(0xA7, LTRACE("AND A"); and_r(a));
        INSTR(0xA8, LTRACE("XOR B"); xor_r(b));
        INSTR(0xA9, LTRACE("XOR C"); xor_r(c));
        INSTR(0xAA, LTRACE("XOR D"); xor_r(d));
        INSTR(0xAB, LTRACE("XOR E"); xor_r(e));
        INSTR(0xAC, LTRACE("XOR H"); xor_r(h));
        INSTR(0xAD, LTRACE("XOR L"); xor_r(l));
        INSTR(0xAE, xor_dhl());
        INSTR(0xAF, LTRACE("XOR A"); xor_r(a));
        INSTR(0xB0, LTRACE("OR B"); or_r(b));
        INSTR(0xB1, LTRACE("OR C"); or_r(c));
        INSTR(0xB2, LTRACE("OR D"); or_r(d));
        INSTR(0xB3, LTRACE("OR E"); or_r(e));
        INSTR(0xB4, LTRACE("OR H"); or_r(h));
        INSTR(0xB5, LTRACE("OR L"); or_r(l));
        INSTR(0xB6, or_dhl());
        INSTR(0xB7, LTRACE("OR A"); or_r(a));
        INSTR(0xB8, LTRACE("CP R"); cp_r(b));
        INSTR(0xB9, LTRACE("CP R"); cp_r(c));
        INSTR(0xBA, LTRACE("CP R"); cp_r(d));
        INSTR(0xBB, LTRACE("CP R"); cp_r(e));
        INSTR(0xBC, LTRACE("CP R"); cp_r(h));
        INSTR(0xBD, LTRACE("CP R"); cp_r(l));
        INSTR(0xBE, cp_dhl());
        INSTR(0xBF, LTRACE("CP A"); cp_r(a));
        INSTR(0xC1, pop_bc());
        INSTR(0xC2, jp_nz_a16());
        INSTR(0xC3, jp_a16());
        INSTR(0xC4, call_nz_a16());
        INSTR(0xC5, push_bc());
        INSTR(0xC6, add_a_d8());
        INSTR(0xC8, ret_z());
        INSTR(0xC9, ret());
        INSTR(0xCD, call_a16());
        INSTR(0xCE, adc_a_d8());
        INSTR(0xD0, ret_nc());
        INSTR(0xD1, pop_de());
        INSTR(0xD5, push_de());
        INSTR(0xD6, sub_d8());
        INSTR(0xD8, ret_c());
        INSTR(0xDE, sbc_a_d8());
        INSTR(0xE0, ldh_da8_a());
        INSTR(0xE1, pop_hl());
        INSTR(0xE2, ld_dc_a());
        INSTR(0xE5, push_hl());
        INSTR(0xE6, and_d8());
        INSTR(0xE9, jp_hl());
        INSTR(0xEA, ld_da16_a());
        INSTR(0xEE, xor_d8());
        INSTR(0xF0, ldh_a_da8());
        INSTR(0xF1, pop_af());
        INSTR(0xF3, di());
        INSTR(0xF5, push_af());
        INSTR(0xF6, or_d8());
        INSTR(0xF9, ld_sp_hl());
        INSTR(0xFA, ld_a_da16());
        INSTR(0xFB, ei());
        INSTR(0xFE, cp_d8());

        case 0xD3:
        case 0xDB:
        case 0xDD:
        case 0xE3:
        case 0xE4:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xF4:
        case 0xFC:
        case 0xFD:
            DumpRegisters();
            mmu.DumpMemoryToFile();
            LFATAL("illegal opcode 0x%02X at 0x%04X", opcode, pc_at_opcode);
            return false;
        default:
            DumpRegisters();
            mmu.DumpMemoryToFile();
            LFATAL("unimplemented opcode 0x%02X at 0x%04X", opcode, pc_at_opcode);
            return false;
    }

    return true;
}

bool SM83::ExecuteCBOpcode(const u8 opcode, u16 pc_at_opcode) {
    switch (opcode) {
        INSTR(0x00, LTRACE("RLC B"); rlc_r(&b));
        INSTR(0x01, LTRACE("RLC C"); rlc_r(&c));
        INSTR(0x02, LTRACE("RLC D"); rlc_r(&d));
        INSTR(0x03, LTRACE("RLC E"); rlc_r(&e));
        INSTR(0x04, LTRACE("RLC H"); rlc_r(&h));
        INSTR(0x05, LTRACE("RLC L"); rlc_r(&l));
        INSTR(0x06, rlc_dhl());
        INSTR(0x07, LTRACE("RLC A"); rlc_r(&a));
        INSTR(0x08, LTRACE("RRC B"); rrc_r(&b));
        INSTR(0x09, LTRACE("RRC C"); rrc_r(&c));
        INSTR(0x0A, LTRACE("RRC D"); rrc_r(&d));
        INSTR(0x0B, LTRACE("RRC E"); rrc_r(&e));
        INSTR(0x0C, LTRACE("RRC H"); rrc_r(&h));
        INSTR(0x0D, LTRACE("RRC L"); rrc_r(&l));
        INSTR(0x0E, rrc_dhl());
        INSTR(0x0F, LTRACE("RRC A"); rrc_r(&a));
        INSTR(0x10, LTRACE("RL B"); rl_r(&b));
        INSTR(0x11, LTRACE("RL C"); rl_r(&c));
        INSTR(0x12, LTRACE("RL D"); rl_r(&d));
        INSTR(0x13, LTRACE("RL E"); rl_r(&e));
        INSTR(0x14, LTRACE("RL H"); rl_r(&h));
        INSTR(0x15, LTRACE("RL L"); rl_r(&l));
        INSTR(0x16, rl_dhl());
        INSTR(0x17, LTRACE("RL A"); rl_r(&a));
        INSTR(0x18, LTRACE("RR B"); rr_r(&b));
        INSTR(0x19, LTRACE("RR C"); rr_r(&c));
        INSTR(0x1A, LTRACE("RR D"); rr_r(&d));
        INSTR(0x1B, LTRACE("RR E"); rr_r(&e));
        INSTR(0x1C, LTRACE("RR H"); rr_r(&h));
        INSTR(0x1D, LTRACE("RR L"); rr_r(&l));
        INSTR(0x1E, rr_dhl());
        INSTR(0x1F, LTRACE("RR A"); rr_r(&a));
        INSTR(0x20, LTRACE("SLA B"); sla_r(&b));
        INSTR(0x21, LTRACE("SLA C"); sla_r(&c));
        INSTR(0x22, LTRACE("SLA D"); sla_r(&d));
        INSTR(0x23, LTRACE("SLA E"); sla_r(&e));
        INSTR(0x24, LTRACE("SLA H"); sla_r(&h));
        INSTR(0x25, LTRACE("SLA L"); sla_r(&l));
        INSTR(0x26, sla_dhl());
        INSTR(0x27, LTRACE("SLA A"); sla_r(&a));
        INSTR(0x28, LTRACE("SRA B"); sra_r(&b));
        INSTR(0x29, LTRACE("SRA C"); sra_r(&c));
        INSTR(0x2A, LTRACE("SRA D"); sra_r(&d));
        INSTR(0x2B, LTRACE("SRA E"); sra_r(&e));
        INSTR(0x2C, LTRACE("SRA H"); sra_r(&h));
        INSTR(0x2D, LTRACE("SRA L"); sra_r(&l));
        INSTR(0x2E, sra_dhl());
        INSTR(0x2F, LTRACE("SRA A"); sra_r(&a));
        INSTR(0x30, LTRACE("SWAP B"); swap_r(&b));
        INSTR(0x31, LTRACE("SWAP C"); swap_r(&c));
        INSTR(0x32, LTRACE("SWAP D"); swap_r(&d));
        INSTR(0x33, LTRACE("SWAP E"); swap_r(&e));
        INSTR(0x34, LTRACE("SWAP H"); swap_r(&h));
        INSTR(0x35, LTRACE("SWAP L"); swap_r(&l));
        INSTR(0x36, swap_dhl());
        INSTR(0x37, LTRACE("SWAP A"); swap_r(&a));
        INSTR(0x38, LTRACE("SRL B"); srl_r(&b));
        INSTR(0x39, LTRACE("SRL C"); srl_r(&c));
        INSTR(0x3A, LTRACE("SRL D"); srl_r(&d));
        INSTR(0x3B, LTRACE("SRL E"); srl_r(&e));
        INSTR(0x3C, LTRACE("SRL H"); srl_r(&h));
        INSTR(0x3D, LTRACE("SRL L"); srl_r(&l));
        INSTR(0x3E, srl_dhl());
        INSTR(0x3F, LTRACE("SRL A"); srl_r(&a));
        INSTR(0x40, LTRACE("BIT 0, B"); bit(0, &b));
        INSTR(0x41, LTRACE("BIT 0, C"); bit(0, &c));
        INSTR(0x42, LTRACE("BIT 0, D"); bit(0, &d));
        INSTR(0x43, LTRACE("BIT 0, E"); bit(0, &e));
        INSTR(0x44, LTRACE("BIT 0, H"); bit(0, &h));
        INSTR(0x45, LTRACE("BIT 0, L"); bit(0, &l));
        INSTR(0x46, bit_dhl(0));
        INSTR(0x47, LTRACE("BIT 0, A"); bit(0, &a));
        INSTR(0x48, LTRACE("BIT 1, B"); bit(1, &b));
        INSTR(0x49, LTRACE("BIT 1, C"); bit(1, &c));
        INSTR(0x4A, LTRACE("BIT 1, D"); bit(1, &d));
        INSTR(0x4B, LTRACE("BIT 1, E"); bit(1, &e));
        INSTR(0x4C, LTRACE("BIT 1, H"); bit(1, &h));
        INSTR(0x4D, LTRACE("BIT 1, L"); bit(1, &l));
        INSTR(0x4E, bit_dhl(1));
        INSTR(0x4F, LTRACE("BIT 1, A"); bit(1, &a));
        INSTR(0x50, LTRACE("BIT 2, B"); bit(2, &b));
        INSTR(0x51, LTRACE("BIT 2, C"); bit(2, &c));
        INSTR(0x52, LTRACE("BIT 2, D"); bit(2, &d));
        INSTR(0x53, LTRACE("BIT 2, E"); bit(2, &e));
        INSTR(0x54, LTRACE("BIT 2, H"); bit(2, &h));
        INSTR(0x55, LTRACE("BIT 2, L"); bit(2, &l));
        INSTR(0x56, bit_dhl(2));
        INSTR(0x57, LTRACE("BIT 2, A"); bit(2, &a));
        INSTR(0x58, LTRACE("BIT 3, B"); bit(3, &b));
        INSTR(0x59, LTRACE("BIT 3, C"); bit(3, &c));
        INSTR(0x5A, LTRACE("BIT 3, D"); bit(3, &d));
        INSTR(0x5B, LTRACE("BIT 3, E"); bit(3, &e));
        INSTR(0x5C, LTRACE("BIT 3, H"); bit(3, &h));
        INSTR(0x5D, LTRACE("BIT 3, L"); bit(3, &l));
        INSTR(0x5E, bit_dhl(3));
        INSTR(0x5F, LTRACE("BIT 3, A"); bit(3, &a));
        INSTR(0x60, LTRACE("BIT 4, B"); bit(4, &b));
        INSTR(0x61, LTRACE("BIT 4, C"); bit(4, &c));
        INSTR(0x62, LTRACE("BIT 4, D"); bit(4, &d));
        INSTR(0x63, LTRACE("BIT 4, E"); bit(4, &e));
        INSTR(0x64, LTRACE("BIT 4, H"); bit(4, &h));
        INSTR(0x65, LTRACE("BIT 4, L"); bit(4, &l));
        INSTR(0x66, bit_dhl(4));
        INSTR(0x67, LTRACE("BIT 4, A"); bit(4, &a));
        INSTR(0x68, LTRACE("BIT 5, B"); bit(5, &b));
        INSTR(0x69, LTRACE("BIT 5, C"); bit(5, &c));
        INSTR(0x6A, LTRACE("BIT 5, D"); bit(5, &d));
        INSTR(0x6B, LTRACE("BIT 5, E"); bit(5, &e));
        INSTR(0x6C, LTRACE("BIT 5, H"); bit(5, &h));
        INSTR(0x6D, LTRACE("BIT 5, L"); bit(5, &l));
        INSTR(0x6E, bit_dhl(5));
        INSTR(0x6F, LTRACE("BIT 5, A"); bit(5, &a));
        INSTR(0x70, LTRACE("BIT 6, B"); bit(6, &b));
        INSTR(0x71, LTRACE("BIT 6, C"); bit(6, &c));
        INSTR(0x72, LTRACE("BIT 6, D"); bit(6, &d));
        INSTR(0x73, LTRACE("BIT 6, E"); bit(6, &e));
        INSTR(0x74, LTRACE("BIT 6, H"); bit(6, &h));
        INSTR(0x75, LTRACE("BIT 6, L"); bit(6, &l));
        INSTR(0x76, bit_dhl(6));
        INSTR(0x77, LTRACE("BIT 6, A"); bit(6, &a));
        INSTR(0x78, LTRACE("BIT 7, B"); bit(7, &b));
        INSTR(0x79, LTRACE("BIT 7, C"); bit(7, &c));
        INSTR(0x7A, LTRACE("BIT 7, D"); bit(7, &d));
        INSTR(0x7B, LTRACE("BIT 7, E"); bit(7, &e));
        INSTR(0x7C, LTRACE("BIT 7, H"); bit(7, &h));
        INSTR(0x7D, LTRACE("BIT 7, L"); bit(7, &l));
        INSTR(0x7E, bit_dhl(7));
        INSTR(0x7F, LTRACE("BIT 7, A"); bit(7, &a));
        INSTR(0x80, LTRACE("RES 0, B"); res(0, &b));
        INSTR(0x81, LTRACE("RES 0, C"); res(0, &c));
        INSTR(0x82, LTRACE("RES 0, D"); res(0, &d));
        INSTR(0x83, LTRACE("RES 0, E"); res(0, &e));
        INSTR(0x84, LTRACE("RES 0, H"); res(0, &h));
        INSTR(0x85, LTRACE("RES 0, L"); res(0, &l));
        INSTR(0x86, res_dhl(0));
        INSTR(0x87, LTRACE("RES 0, A"); res(0, &a));
        INSTR(0x88, LTRACE("RES 1, B"); res(1, &b));
        INSTR(0x89, LTRACE("RES 1, C"); res(1, &c));
        INSTR(0x8A, LTRACE("RES 1, D"); res(1, &d));
        INSTR(0x8B, LTRACE("RES 1, E"); res(1, &e));
        INSTR(0x8C, LTRACE("RES 1, H"); res(1, &h));
        INSTR(0x8D, LTRACE("RES 1, L"); res(1, &l));
        INSTR(0x8E, res_dhl(1));
        INSTR(0x8F, LTRACE("RES 1, A"); res(1, &a));
        INSTR(0x90, LTRACE("RES 2, B"); res(2, &b));
        INSTR(0x91, LTRACE("RES 2, C"); res(2, &c));
        INSTR(0x92, LTRACE("RES 2, D"); res(2, &d));
        INSTR(0x93, LTRACE("RES 2, E"); res(2, &e));
        INSTR(0x94, LTRACE("RES 2, H"); res(2, &h));
        INSTR(0x95, LTRACE("RES 2, L"); res(2, &l));
        INSTR(0x96, res_dhl(2));
        INSTR(0x97, LTRACE("RES 2, A"); res(2, &a));
        INSTR(0x98, LTRACE("RES 3, B"); res(3, &b));
        INSTR(0x99, LTRACE("RES 3, C"); res(3, &c));
        INSTR(0x9A, LTRACE("RES 3, D"); res(3, &d));
        INSTR(0x9B, LTRACE("RES 3, E"); res(3, &e));
        INSTR(0x9C, LTRACE("RES 3, H"); res(3, &h));
        INSTR(0x9D, LTRACE("RES 3, L"); res(3, &l));
        INSTR(0x9E, res_dhl(3));
        INSTR(0x9F, LTRACE("RES 3, A"); res(3, &a));
        INSTR(0xA0, LTRACE("RES 4, B"); res(4, &b));
        INSTR(0xA1, LTRACE("RES 4, C"); res(4, &c));
        INSTR(0xA2, LTRACE("RES 4, D"); res(4, &d));
        INSTR(0xA3, LTRACE("RES 4, E"); res(4, &e));
        INSTR(0xA4, LTRACE("RES 4, H"); res(4, &h));
        INSTR(0xA5, LTRACE("RES 4, L"); res(4, &l));
        INSTR(0xA6, res_dhl(4));
        INSTR(0xA7, LTRACE("RES 4, A"); res(4, &a));
        INSTR(0xA8, LTRACE("RES 5, B"); res(5, &b));
        INSTR(0xA9, LTRACE("RES 5, C"); res(5, &c));
        INSTR(0xAA, LTRACE("RES 5, D"); res(5, &d));
        INSTR(0xAB, LTRACE("RES 5, E"); res(5, &e));
        INSTR(0xAC, LTRACE("RES 5, H"); res(5, &h));
        INSTR(0xAD, LTRACE("RES 5, L"); res(5, &l));
        INSTR(0xAE, res_dhl(5));
        INSTR(0xAF, LTRACE("RES 5, A"); res(5, &a));
        INSTR(0xB0, LTRACE("RES 6, B"); res(6, &b));
        INSTR(0xB1, LTRACE("RES 6, C"); res(6, &c));
        INSTR(0xB2, LTRACE("RES 6, D"); res(6, &d));
        INSTR(0xB3, LTRACE("RES 6, E"); res(6, &e));
        INSTR(0xB4, LTRACE("RES 6, H"); res(6, &h));
        INSTR(0xB5, LTRACE("RES 6, L"); res(6, &l));
        INSTR(0xB6, res_dhl(6));
        INSTR(0xB7, LTRACE("RES 6, A"); res(6, &a));
        INSTR(0xB8, LTRACE("RES 7, B"); res(7, &b));
        INSTR(0xB9, LTRACE("RES 7, C"); res(7, &c));
        INSTR(0xBA, LTRACE("RES 7, D"); res(7, &d));
        INSTR(0xBB, LTRACE("RES 7, E"); res(7, &e));
        INSTR(0xBC, LTRACE("RES 7, H"); res(7, &h));
        INSTR(0xBD, LTRACE("RES 7, L"); res(7, &l));
        INSTR(0xBE, res_dhl(7));
        INSTR(0xBF, LTRACE("RES 7, A"); res(7, &a));
        INSTR(0xC0, LTRACE("SET 0, B"); set(0, &b));
        INSTR(0xC1, LTRACE("SET 0, C"); set(0, &c));
        INSTR(0xC2, LTRACE("SET 0, D"); set(0, &d));
        INSTR(0xC3, LTRACE("SET 0, E"); set(0, &e));
        INSTR(0xC4, LTRACE("SET 0, H"); set(0, &h));
        INSTR(0xC5, LTRACE("SET 0, L"); set(0, &l));
        INSTR(0xC6, set_dhl(0));
        INSTR(0xC7, LTRACE("SET 0, A"); set(0, &a));
        INSTR(0xC8, LTRACE("SET 1, B"); set(1, &b));
        INSTR(0xC9, LTRACE("SET 1, C"); set(1, &c));
        INSTR(0xCA, LTRACE("SET 1, D"); set(1, &d));
        INSTR(0xCB, LTRACE("SET 1, E"); set(1, &e));
        INSTR(0xCC, LTRACE("SET 1, H"); set(1, &h));
        INSTR(0xCD, LTRACE("SET 1, L"); set(1, &l));
        INSTR(0xCE, set_dhl(1));
        INSTR(0xCF, LTRACE("SET 1, A"); set(1, &a));
        INSTR(0xD0, LTRACE("SET 2, B"); set(2, &b));
        INSTR(0xD1, LTRACE("SET 2, C"); set(2, &c));
        INSTR(0xD2, LTRACE("SET 2, D"); set(2, &d));
        INSTR(0xD3, LTRACE("SET 2, E"); set(2, &e));
        INSTR(0xD4, LTRACE("SET 2, H"); set(2, &h));
        INSTR(0xD5, LTRACE("SET 2, L"); set(2, &l));
        INSTR(0xD6, set_dhl(2));
        INSTR(0xD7, LTRACE("SET 2, A"); set(2, &a));
        INSTR(0xD8, LTRACE("SET 3, B"); set(3, &b));
        INSTR(0xD9, LTRACE("SET 3, C"); set(3, &c));
        INSTR(0xDA, LTRACE("SET 3, D"); set(3, &d));
        INSTR(0xDB, LTRACE("SET 3, E"); set(3, &e));
        INSTR(0xDC, LTRACE("SET 3, H"); set(3, &h));
        INSTR(0xDD, LTRACE("SET 3, L"); set(3, &l));
        INSTR(0xDE, set_dhl(3));
        INSTR(0xDF, LTRACE("SET 3, A"); set(3, &a));
        INSTR(0xE0, LTRACE("SET 4, B"); set(4, &b));
        INSTR(0xE1, LTRACE("SET 4, C"); set(4, &c));
        INSTR(0xE2, LTRACE("SET 4, D"); set(4, &d));
        INSTR(0xE3, LTRACE("SET 4, E"); set(4, &e));
        INSTR(0xE4, LTRACE("SET 4, H"); set(4, &h));
        INSTR(0xE5, LTRACE("SET 4, L"); set(4, &l));
        INSTR(0xE6, set_dhl(4));
        INSTR(0xE7, LTRACE("SET 4, A"); set(4, &a));
        INSTR(0xE8, LTRACE("SET 5, B"); set(5, &b));
        INSTR(0xE9, LTRACE("SET 5, C"); set(5, &c));
        INSTR(0xEA, LTRACE("SET 5, D"); set(5, &d));
        INSTR(0xEB, LTRACE("SET 5, E"); set(5, &e));
        INSTR(0xEC, LTRACE("SET 5, H"); set(5, &h));
        INSTR(0xED, LTRACE("SET 5, L"); set(5, &l));
        INSTR(0xEE, set_dhl(5));
        INSTR(0xEF, LTRACE("SET 5, A"); set(5, &a));
        INSTR(0xF0, LTRACE("SET 6, B"); set(6, &b));
        INSTR(0xF1, LTRACE("SET 6, C"); set(6, &c));
        INSTR(0xF2, LTRACE("SET 6, D"); set(6, &d));
        INSTR(0xF3, LTRACE("SET 6, E"); set(6, &e));
        INSTR(0xF4, LTRACE("SET 6, H"); set(6, &h));
        INSTR(0xF5, LTRACE("SET 6, L"); set(6, &l));
        INSTR(0xF6, set_dhl(6));
        INSTR(0xF7, LTRACE("SET 6, A"); set(6, &a));
        INSTR(0xF8, LTRACE("SET 7, B"); set(7, &b));
        INSTR(0xF9, LTRACE("SET 7, C"); set(7, &c));
        INSTR(0xFA, LTRACE("SET 7, D"); set(7, &d));
        INSTR(0xFB, LTRACE("SET 7, E"); set(7, &e));
        INSTR(0xFC, LTRACE("SET 7, H"); set(7, &h));
        INSTR(0xFD, LTRACE("SET 7, L"); set(7, &l));
        INSTR(0xFE, set_dhl(7));
        INSTR(0xFF, LTRACE("SET 7, A"); set(7, &a));
#undef INSTR
        default:
            DumpRegisters();
            mmu.DumpMemoryToFile();
            LFATAL("unimplemented CB opcode 0x%02X at 0x%04X", opcode, pc_at_opcode);
            return false;
    }

    return true;
}

void SM83::HandleInterrupts() {
    if (!interrupts_enabled) {
        return;
    }

    LERROR("TODO: interrupts!");
}

void SM83::DumpRegisters() {
    LFATAL("AF=%04X BC=%04X DE=%04X HL=%04X SP=%04X PC=%04X", af, bc, de, hl, sp, pc_at_opcode);
    if (!f) {
        LFATAL("Flags: none");
    } else {
        LFATAL("Flags: [%s%s%s%s]", (HasFlag(Flags::Zero)) ? "Z" : " ",
                                    (HasFlag(Flags::Negate)) ? "N" : " ",
                                    (HasFlag(Flags::HalfCarry)) ? "H" : " ",
                                    (HasFlag(Flags::Carry)) ? "C" : " ");
    }
}

void SM83::adc_a_d8() {
    u8 value = GetByteFromPC();
    LTRACE("ADC A, 0x%02X", value);

    bool carry = HasFlag(Flags::Carry);
    u16 full = a + value + carry;
    u8 result = static_cast<u8>(full);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(((a & 0xF) + (value & 0xF) + carry) > 0xF);
    SetCarryFlag(full > 0xFF);

    a = result;
}

void SM83::adc_a_r(u8 reg) {
    bool carry = HasFlag(Flags::Carry);
    u16 full = a + reg + carry;
    u8 result = static_cast<u8>(full);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(((a & 0xF) + (reg & 0xF) + carry) > 0xF);
    SetCarryFlag(full > 0xFF);

    a = result;
}

void SM83::add_a_d8() {
    u8 value = GetByteFromPC();
    LTRACE("ADD A, (0x%02X)", value);
    u16 result = a + value;

    SetZeroFlag(static_cast<u8>(result) == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((a & 0xF) + (value & 0xF) > 0xF);
    SetCarryFlag(result > 0xFF);

    a = static_cast<u8>(result);
}

void SM83::add_a_dhl() {
    LTRACE("ADD A, (HL)");
    u8 value = mmu.Read8(hl);
    u16 result = a + value;

    SetZeroFlag(static_cast<u8>(result) == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((a & 0xF) + (value & 0xF) > 0xF);
    SetCarryFlag(result > 0xFF);

    a = static_cast<u8>(result);
}

void SM83::add_a_r(u8 reg) {
    u16 result = a + reg;

    SetZeroFlag(static_cast<u8>(result) == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((a & 0xF) + (reg & 0xF) > 0xF);
    SetCarryFlag(result > 0xFF);

    a = static_cast<u8>(result);
}

void SM83::add_hl_bc() {
    LTRACE("ADD HL, BC");
    u32 result = hl + bc;

    SetNegateFlag(false);
    SetHalfCarryFlag(((hl & 0xFFF) + (bc & 0xFFF)) & 0x1000);
    SetCarryFlag((result & 0x10000) != 0);

    hl = static_cast<u16>(result);
}

void SM83::add_hl_de() {
    LTRACE("ADD HL, DE");
    u32 result = hl + de;

    SetNegateFlag(false);
    SetHalfCarryFlag(((hl & 0xFFF) + (de & 0xFFF)) & 0x1000);
    SetCarryFlag((result & 0x10000) != 0);

    hl = static_cast<u16>(result);
}

void SM83::add_hl_hl() {
    LTRACE("ADD HL, HL");
    u32 result = hl + hl;

    SetNegateFlag(false);
    SetHalfCarryFlag(((hl & 0xFFF) + (hl & 0xFFF)) & 0x1000);
    SetCarryFlag((result & 0x10000) != 0);

    hl = static_cast<u16>(result);
}

void SM83::and_d8() {
    u8 value = GetByteFromPC();
    LTRACE("AND 0x%02X", value);

    a &= value;

    SetZeroFlag(a == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(true);
    SetCarryFlag(false);
}

void SM83::and_r(u8 reg) {
    a &= reg;

    SetZeroFlag(a == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(true);
    SetCarryFlag(false);
}

void SM83::bit(u8 bit, u8* reg) {
    SetZeroFlag(!(*reg & (1 << bit)));
    SetNegateFlag(false);
    SetHalfCarryFlag(true);
}

void SM83::bit_dhl(u8 bit) {
    LTRACE("BIT %u, (HL)", bit);
    u8 value = mmu.Read8(hl);

    SetZeroFlag(!(value & (1 << bit)));
    SetNegateFlag(false);
    SetHalfCarryFlag(true);
}

void SM83::call_a16() {
    u16 address = GetWordFromPC();
    LTRACE("CALL 0x%04X", address);

    StackPush(&pc);
    pc = address;
}

void SM83::call_nz_a16() {
    u16 address = GetWordFromPC();
    LTRACE("CALL NZ, 0x%04X", address);

    if (!HasFlag(Flags::Zero)) {
        StackPush(&pc);
        pc = address;
    }
}

void SM83::ccf() {
    LTRACE("CCF");

    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(!HasFlag(Flags::Carry));
}

void SM83::cp_d8() {
    u8 value = GetByteFromPC();
    LTRACE("CP A, 0x%02X", value);

    SetZeroFlag(a == value);
    SetNegateFlag(true);
    SetHalfCarryFlag((a & 0xF) < (value & 0xF));
    SetCarryFlag(a < value);
}

void SM83::cp_dhl() {
    LTRACE("CP (HL)");

    u8 value = mmu.Read8(hl);

    SetZeroFlag(a == value);
    SetNegateFlag(true);
    SetHalfCarryFlag((a & 0xF) < (value & 0xF));
    SetCarryFlag(a < value);
}

void SM83::cp_r(u8 reg) {
    SetZeroFlag(a == reg);
    SetNegateFlag(true);
    SetHalfCarryFlag((a & 0xF) < (reg & 0xF));
    SetCarryFlag(a < reg);
}

void SM83::cpl() {
    LTRACE("CPL");

    a = ~a;

    SetNegateFlag(true);
    SetHalfCarryFlag(true);
}

void SM83::daa() {
    LTRACE("DAA");
    // lifted from SameBoy.
    // this is one nutty instruction
    // TODO: come up with an original implementation

    uint16_t result = a;
    a = 0;

    if (HasFlag(Flags::Negate)) {
        if (HasFlag(Flags::HalfCarry)) {
            result -= 0x06;
            result &= 0xFF;
        }

        if (HasFlag(Flags::Carry)) {
            result -= 0x60;
        }
    } else {
        if (HasFlag(Flags::HalfCarry) || (result & 0x0F) >= 0x0A) {
            result += 0x06;
        }

        if (HasFlag(Flags::Carry) || result >= 0xA0) {
            result += 0x60;
        }
    }

    SetZeroFlag((result & 0xFF) == 0);
    SetHalfCarryFlag(false);
    if ((result & 0x100) == 0x100) {
        // don't set carry flag to false if the result isn't 16 bit
        SetCarryFlag(true);
    }

    a |= result;
}

void SM83::dec_a() {
    LTRACE("DEC A");

    a--;

    SetZeroFlag(a == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((a & 0x0F) == 0x0F);
}

void SM83::dec_b() {
    LTRACE("DEC B");

    b--;

    SetZeroFlag(b == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((b & 0x0F) == 0x0F);
}

void SM83::dec_bc() {
    LTRACE("DEC BC");
    bc--;
}

void SM83::dec_c() {
    LTRACE("DEC C");

    c--;

    SetZeroFlag(c == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((c & 0x0F) == 0x0F);
}

void SM83::dec_d() {
    LTRACE("DEC D");

    d--;

    SetZeroFlag(d == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((d & 0x0F) == 0x0F);
}

void SM83::dec_de() {
    LTRACE("DEC DE");
    de--;
}

void SM83::dec_dhl() {
    LTRACE("DEC (HL)");

    u8 value = mmu.Read8(hl);
    value--;
    mmu.Write8(hl, value);

    SetZeroFlag(value == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((value & 0x0F) == 0x0F);
}

void SM83::dec_e() {
    LTRACE("DEC E");

    e--;

    SetZeroFlag(e == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((e & 0x0F) == 0x0F);
}

void SM83::dec_h() {
    LTRACE("DEC H");

    h--;

    SetZeroFlag(h == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((h & 0x0F) == 0x0F);
}

void SM83::dec_hl() {
    LTRACE("DEC HL");
    hl--;
}

void SM83::dec_l() {
    LTRACE("DEC L");

    l--;

    SetZeroFlag(l == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag((l & 0x0F) == 0x0F);
}

void SM83::di() {
    LTRACE("DI");
    interrupts_enabled = false;
    LINFO("interrupts have been disabled");
}

void SM83::ei() {
    LTRACE("EI");
    interrupts_enabled = true;
    LINFO("interrupts have been enabled");
}

void SM83::inc_a() {
    LTRACE("INC A");

    a++;

    SetZeroFlag(a == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((a & 0x0F) == 0x00);
}

void SM83::inc_b() {
    LTRACE("INC B");

    b++;

    SetZeroFlag(b == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((b & 0x0F) == 0x00);
}

void SM83::inc_bc() {
    LTRACE("INC BC");
    bc++;
}

void SM83::inc_c() {
    LTRACE("INC C");

    c++; // lol get it

    SetZeroFlag(c == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((c & 0x0F) == 0x00);
}

void SM83::inc_d() {
    LTRACE("INC D");

    d++;

    SetZeroFlag(d == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((d & 0x0F) == 0x00);
}

void SM83::inc_de() {
    LTRACE("INC DE");
    de++;
}

void SM83::inc_dhl() {
    LTRACE("INC (HL)");

    u8 value = mmu.Read8(hl);
    value++;
    mmu.Write8(hl, value);

    SetZeroFlag(value == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((value & 0x0F) == 0x00);
}

void SM83::inc_e() {
    LTRACE("INC E");

    e++;

    SetZeroFlag(e == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((e & 0x0F) == 0x00);
}

void SM83::inc_h() {
    LTRACE("INC H");

    h++;

    SetZeroFlag(h == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((h & 0x0F) == 0x00);
}

void SM83::inc_hl() {
    LTRACE("INC HL");
    hl++;
}

void SM83::inc_l() {
    LTRACE("INC L");

    l++;

    SetZeroFlag(l == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag((l & 0x0F) == 0x00);
}

void SM83::inc_sp() {
    LTRACE("INC SP");
    sp++;
}

void SM83::jp_a16() {
    u16 addr = GetWordFromPC();
    LTRACE("JP 0x%04X", addr);

    pc = addr;
}

void SM83::jp_hl() {
    LTRACE("JP HL");
    pc = hl;
}

void SM83::jp_nz_a16() {
    u16 addr = GetWordFromPC();
    LTRACE("JP NZ, 0x%04X", addr);

    if (!HasFlag(Flags::Zero)) {
        pc = addr;
    }
}

void SM83::jr_r8() {
    s8 offset = static_cast<s8>(GetByteFromPC());
    u16 new_pc = static_cast<u16>(pc + offset);
    LTRACE("JR 0x%04X", new_pc);

    pc = new_pc;
}

void SM83::jr_c_r8() {
    s8 offset = static_cast<s8>(GetByteFromPC());
    u16 potential_pc = pc + offset;
    LTRACE("JR C, 0x%04X", potential_pc);

    if (HasFlag(Flags::Carry)) {
        pc = potential_pc;
    }
}

void SM83::jr_nc_r8() {
    s8 offset = static_cast<s8>(GetByteFromPC());
    u16 potential_pc = pc + offset;
    LTRACE("JR NC, 0x%04X", potential_pc);

    if (!HasFlag(Flags::Carry)) {
        pc = potential_pc;
    }
}

void SM83::jr_nz_r8() {
    s8 offset = static_cast<s8>(GetByteFromPC());
    u16 potential_pc = pc + offset;
    LTRACE("JR NZ, 0x%04X", potential_pc);

    if (!HasFlag(Flags::Zero)) {
        pc = potential_pc;
    }
}

void SM83::jr_z_r8() {
    s8 offset = static_cast<u8>(GetByteFromPC());
    u16 potential_pc = pc + offset;
    LTRACE("JR Z, 0x%04X", potential_pc);

    if (HasFlag(Flags::Zero)) {
        pc = potential_pc;
    }
}

void SM83::ld_d_a() {
    LTRACE("LD D, A");
    d = a;
}

void SM83::ld_d_b() {
    LTRACE("LD D, B");
    d = b;
}

void SM83::ld_d_c() {
    LTRACE("LD D, C");
    d = c;
}

void SM83::ld_d_d() {
    LTRACE("LD D, D");
    d = d;
}

void SM83::ld_d_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD D, 0x%02X", value);

    d = value;
}

void SM83::ld_d_dhl() {
    LTRACE("LD D, (HL)");
    d = mmu.Read8(hl);
}

void SM83::ld_d_e() {
    LTRACE("LD D, E");
    d = e;
}

void SM83::ld_d_h() {
    LTRACE("LD D, H");
    d = h;
}

void SM83::ld_d_l() {
    LTRACE("LD D, L");
    d = l;
}

void SM83::ld_da16_a() {
    u16 address = GetWordFromPC();
    LTRACE("LD (0x%04X), A", address);

    mmu.Write8(address, a);
}

void SM83::ld_da16_sp() {
    u16 address = GetWordFromPC();
    LTRACE("LD (0x%04X), SP", address);

    mmu.Write16(address, sp);
}

void SM83::ld_dbc_a() {
    LTRACE("LD (BC), A");

    mmu.Write8(bc, a);
}

void SM83::ld_dc_a() {
    LTRACE("LD (0xFF00+C), A");
    u16 address = 0xFF00 + c;

    mmu.Write8(address, a);
}

void SM83::ld_dde_a() {
    LTRACE("LD (DE), A");
    
    mmu.Write8(de, a);
}

void SM83::ld_de_d16() {
    u16 value = GetWordFromPC();
    LTRACE("LD DE, 0x%04X", value);

    de = value;
}

void SM83::ld_dhl_a() {
    LTRACE("LD (HL), A");

    mmu.Write8(hl, a);
}

void SM83::ld_dhl_b() {
    LTRACE("LD (HL), B");

    mmu.Write8(hl, b);
}

void SM83::ld_dhl_c() {
    LTRACE("LD (HL), C");

    mmu.Write8(hl, c);
}

void SM83::ld_dhl_d() {
    LTRACE("LD (HL), D");

    mmu.Write8(hl, d);
}

void SM83::ld_dhl_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD (HL), 0x%02X", value);

    mmu.Write8(hl, value);
}

void SM83::ld_dhl_e() {
    LTRACE("LD (HL), E");
    
    mmu.Write8(hl, e);
}

void SM83::ld_dhl_h() {
    LTRACE("LD (HL), H");
    
    mmu.Write8(hl, h);
}

void SM83::ld_dhl_l() {
    LTRACE("LD (HL), L");
    
    mmu.Write8(hl, l);
}

void SM83::ld_dhld_a() {
    LTRACE("LD (HL-), A");
        
    mmu.Write8(hl--, a);
}

void SM83::ld_dhli_a() {
    LTRACE("LD (HL+), A");

    mmu.Write8(hl++, a);
}

void SM83::ld_a_da16() {
    u16 addr = GetWordFromPC();
    LTRACE("LD A, (0x%04X)", addr);

    a = mmu.Read8(addr);
}

void SM83::ld_a_a() {
    LTRACE("LD A, A");
    a = a;
}

void SM83::ld_a_b() {
    LTRACE("LD A, B");
    a = b;
}

void SM83::ld_a_c() {
    LTRACE("LD A, C");
    a = c;
}

void SM83::ld_a_d() {
    LTRACE("LD A, D");
    a = d;
}

void SM83::ld_a_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD A, 0x%02X", value);

    a = value;
}

void SM83::ld_a_dbc() {
    LTRACE("LD A, (BC)");
    a = mmu.Read8(bc);
}

void SM83::ld_a_dde() {
    LTRACE("LD A, (DE)");
    a = mmu.Read8(de);
}

void SM83::ld_a_dhl() {
    LTRACE("LD A, (HL)");
    a = mmu.Read8(hl);
}

void SM83::ld_a_dhld() {
    LTRACE("LD A, (HL-)");
    a = mmu.Read8(hl--);
}

void SM83::ld_a_dhli() {
    LTRACE("LD A, (HL+)");
    a = mmu.Read8(hl++);
}

void SM83::ld_a_e() {
    LTRACE("LD A, E");
    a = e;
}

void SM83::ld_a_h() {
    LTRACE("LD A, H");
    a = h;
}

void SM83::ld_a_l() {
    LTRACE("LD A, L");
    a = l;
}

void SM83::ld_b_a() {
    LTRACE("LD B, A");
    b = a;
}

void SM83::ld_b_b() {
    LTRACE("LD B, B");
    b = b;
}

void SM83::ld_b_c() {
    LTRACE("LD B, C");
    b = c;
}

void SM83::ld_b_d() {
    LTRACE("LD B, D");
    b = d;
}

void SM83::ld_b_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD B, $%02X", value);

    b = value;
}

void SM83::ld_b_dhl() {
    LTRACE("LD B, (HL)");
    b = mmu.Read8(hl);
}

void SM83::ld_b_e() {
    LTRACE("LD B, E");
    b = e;
}

void SM83::ld_b_h() {
    LTRACE("LD B, H");
    b = h;
}

void SM83::ld_b_l() {
    LTRACE("LD B, L");
    b = l;
}

void SM83::ld_bc_d16() {
    u16 value = GetWordFromPC();
    LTRACE("LD BC, 0x%04X", value);

    bc = value;
}

void SM83::ld_c_a() {
    LTRACE("LD C, A");
    c = a;
}

void SM83::ld_c_b() {
    LTRACE("LD C, B");
    c = b;
}

void SM83::ld_c_c() {
    LTRACE("LD C, C");
    c = c;
}

void SM83::ld_c_d() {
    LTRACE("LD C, D");
    c = d;
}

void SM83::ld_c_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD C, 0x%02X", value);

    c = value;
}

void SM83::ld_c_e() {
    LTRACE("LD C, E");
    c = e;
}

void SM83::ld_c_h() {
    LTRACE("LD C, H");
    c = h;
}

void SM83::ld_c_l() {
    LTRACE("LD C, L");
    c = l;
}

void SM83::ld_c_dhl() {
    LTRACE("LD C, (HL)");
    c = mmu.Read8(hl);
}

void SM83::ld_e_a() {
    LTRACE("LD E, A");
    e = a;
}

void SM83::ld_e_b() {
    LTRACE("LD E, B");
    e = b;
}

void SM83::ld_e_c() {
    LTRACE("LD E, C");
    e = c;
}

void SM83::ld_e_d() {
    LTRACE("LD E, D");
    e = d;
}

void SM83::ld_e_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD E, 0x%02X", value);

    e = value;
}

void SM83::ld_e_dhl() {
    LTRACE("LD E, (HL)");
    e = mmu.Read8(hl);
}

void SM83::ld_e_e() {
    LTRACE("LD E, E");
    e = e;
}

void SM83::ld_e_h() {
    LTRACE("LD E, H");
    e = h;
}

void SM83::ld_e_l() {
    LTRACE("LD E, L");
    e = l;
}

void SM83::ld_h_a() {
    LTRACE("LD H, A");
    h = a;
}

void SM83::ld_h_b() {
    LTRACE("LD H, B");
    h = b;
}

void SM83::ld_h_c() {
    LTRACE("LD H, C");
    h = c;
}

void SM83::ld_h_d() {
    LTRACE("LD H, D");
    h = d;
}

void SM83::ld_h_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD H, 0x%02X", value);

    h = value;
}

void SM83::ld_h_dhl() {
    LTRACE("LD H, (HL)");
    h = mmu.Read8(hl);
}

void SM83::ld_h_e() {
    LTRACE("LD H, E");
    h = e;
}

void SM83::ld_h_h() {
    LTRACE("LD H, H");
    h = h;
}

void SM83::ld_h_l() {
    LTRACE("LD H, L");
    h = l;
}

void SM83::ld_hl_d16() {
    u16 value = GetWordFromPC();
    LTRACE("LD HL, 0x%04X", value);

    hl = value;
}

void SM83::ld_l_a() {
    LTRACE("LD L, A");
    l = a;
}

void SM83::ld_l_b() {
    LTRACE("LD L, B");
    l = b;
}

void SM83::ld_l_c() {
    LTRACE("LD L, C");
    l = c;
}

void SM83::ld_l_d() {
    LTRACE("LD L, D");
    l = d;
}

void SM83::ld_l_d8() {
    u8 value = GetByteFromPC();
    LTRACE("LD L, 0x%02X", value);

    l = value;
}

void SM83::ld_l_dhl() {
    LTRACE("LD L, (HL)");
    l = mmu.Read8(hl);
}

void SM83::ld_l_e() {
    LTRACE("LD L, E");
    l = e;
}

void SM83::ld_l_h() {
    LTRACE("LD L, H");
    l = h;
}

void SM83::ld_l_l() {
    LTRACE("LD L, L");
    l = l;
}

void SM83::ld_sp_d16() {
    u16 value = GetWordFromPC();
    LTRACE("LD SP, 0x%04X", value);

    sp = value;
}

void SM83::ld_sp_hl() {
    LTRACE("LD SP, HL");
    sp = hl;
}

void SM83::ldh_a_da8() {
    u8 offset = GetByteFromPC();
    u16 address = 0xFF00 + offset;
    LTRACE("LDH A, (0xFF00+0x%02X)", offset);

    a = mmu.Read8(address);
}

void SM83::ldh_da8_a() {
    u8 offset = GetByteFromPC();
    u16 address = 0xFF00 + offset;
    LTRACE("LDH (0xFF00+0x%02X), A", offset);

    mmu.Write8(address, a);
}

void SM83::nop() {
    LTRACE("NOP");
}

void SM83::or_d8() {
    u8 value = GetByteFromPC();
    LTRACE("OR 0x%02X", value);

    a |= value;

    SetZeroFlag(a == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);
}

void SM83::or_dhl() {
    LTRACE("OR (HL)");

    a |= mmu.Read8(hl);

    SetZeroFlag(a == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);
}

void SM83::or_r(u8 reg) {
    a |= reg;

    SetZeroFlag(a == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);
}

void SM83::pop_af() {
    LTRACE("POP AF");
    StackPop(&af);

    // TODO: find an actually readable way to do this
    f &= 0xF0;
}

void SM83::pop_bc() {
    LTRACE("POP BC");
    StackPop(&bc);
}

void SM83::pop_de() {
    LTRACE("POP DE");
    StackPop(&de);
}

void SM83::pop_hl() {
    LTRACE("POP HL");
    StackPop(&hl);
}

void SM83::push_af() {
    LTRACE("PUSH AF");
    StackPush(&af);
}

void SM83::push_bc() {
    LTRACE("PUSH BC");
    StackPush(&bc);
}

void SM83::push_de() {
    LTRACE("PUSH DE");
    StackPush(&de);
}

void SM83::push_hl() {
    LTRACE("PUSH HL");
    StackPush(&hl);
}

void SM83::res(u8 bit, u8* reg) {
    *reg &= ~(1 << bit);
}

void SM83::res_dhl(u8 bit) {
    LTRACE("RES %u, (HL)", bit);
    u8 value = mmu.Read8(hl);

    value &= ~(1 << bit);
    mmu.Write8(hl, value);
}

void SM83::ret() {
    LTRACE("RET");
    StackPop(&pc);
}

void SM83::ret_c() {
    LTRACE("RET C");

    if (HasFlag(Flags::Carry)) {
        StackPop(&pc);
    }
}

void SM83::ret_nc() {
    LTRACE("RET NC");
    
    if (!HasFlag(Flags::Carry)) {
        StackPop(&pc);
    }
}

void SM83::ret_z() {
    LTRACE("RET Z");

    if (HasFlag(Flags::Zero)) {
        StackPop(&pc);
    }
}

void SM83::rl_dhl() {
    LTRACE("RL (HL)");

    u8 value = mmu.Read8(hl);
    bool carry = HasFlag(Flags::Carry);

    bool should_carry = value & (1 << 7);
    SetCarryFlag(should_carry);

    u8 result = value << 1;
    result |= carry;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);

    mmu.Write8(hl, result);
}

void SM83::rl_r(u8* reg) {
    bool carry = HasFlag(Flags::Carry);

    bool should_carry = *reg & (1 << 7);
    SetCarryFlag(should_carry);

    u8 result = *reg << 1;
    result |= carry;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);

    *reg = result;
}

void SM83::rla() {
    LTRACE("RLA");

    bool carry = HasFlag(Flags::Carry);

    bool should_carry = a & (1 << 7);
    SetCarryFlag(should_carry);

    u8 result = a << 1;
    result |= carry;

    SetZeroFlag(false);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);

    a = result;
}

void SM83::rlc_dhl() {
    LTRACE("RLC (HL)");

    u8 reg = mmu.Read8(hl);
    u8 result = reg << 1;
    bool should_carry = reg & (1 << 7);

    SetZeroFlag(reg == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    mmu.Write8(hl, result | should_carry);
}

void SM83::rlc_r(u8* reg) {
    u8 result = *reg << 1;
    bool should_carry = *reg & (1 << 7);

    SetZeroFlag(*reg == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    *reg = (result | should_carry);
}

void SM83::rlca() {
    LTRACE("RLCA");
    u8 result = a << 1;
    bool should_carry = a & (1 << 7);

    SetZeroFlag(false);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    a = (result | should_carry);
}

void SM83::rr_dhl() {
    LTRACE("RR (HL)");

    u8 value = mmu.Read8(hl);
    bool carry = HasFlag(Flags::Carry);

    SetCarryFlag(value & 0x1);

    u8 result = value >> 1;
    result |= (carry << 7);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);

    mmu.Write8(hl, result);
}

void SM83::rr_r(u8* reg) {
    bool carry = HasFlag(Flags::Carry);

    SetCarryFlag(*reg & 0x1);

    u8 result = *reg >> 1;
    result |= (carry << 7);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);

    *reg = result;
}

void SM83::rra() {
    LTRACE("RRA");
    bool carry = HasFlag(Flags::Carry);

    u8 result = a >> 1;
    result |= (carry << 7);

    SetZeroFlag(false);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(a & 0x1);

    a = result;
}

void SM83::rrc_dhl() {
    LTRACE("RRC (HL)");

    u8 reg = mmu.Read8(hl);
    bool should_carry = reg & 0x1;
    u8 result = (reg >> 1) | (should_carry << 7);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    mmu.Write8(hl, result);
}

void SM83::rrc_r(u8* reg) {
    bool should_carry = *reg & 0x1;
    u8 result = (*reg >> 1) | (should_carry << 7);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    *reg = result;
}

void SM83::rrca() {
    LTRACE("RRCA");
    bool carry = a & 0x1;
    u8 result = static_cast<u8>((a >> 1) | (carry << 7));

    SetZeroFlag(false);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(carry);

    a = result;
}

void SM83::swap_dhl() {
    LTRACE("SWAP (HL)");

    u8 value = mmu.Read8(hl);
    u8 low = value & 0x0F;
    u8 high = (value & 0xF0) >> 4;

    u8 result = static_cast<u8>((low << 4) | high);

    mmu.Write8(hl, result);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);
}

void SM83::swap_r(u8* reg) {
    u8 low = *reg & 0x0F;
    u8 high = (*reg & 0xF0) >> 4;

    u8 result = static_cast<u8>((low << 4) | high);

    *reg = result;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);
}

void SM83::sbc_a_d8() {
    u8 value = GetByteFromPC();
    LTRACE("SBC A, 0x%02X", value);

    bool carry = HasFlag(Flags::Carry);
    u16 full = a - value - carry;
    u8 result = static_cast<u8>(full);

    SetZeroFlag(result == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag(((a & 0xF) < (value & 0xF) + carry) != 0);
    SetCarryFlag(full > 0xFF);

    a = result;
}

void SM83::sbc_r(u8 reg) {
    bool carry = HasFlag(Flags::Carry);
    u16 full = a - reg - carry;
    u8 result = static_cast<u8>(full);

    SetZeroFlag(result == 0);
    SetNegateFlag(true);
    SetHalfCarryFlag(((a & 0xF) < (reg & 0xF) + carry) != 0);
    SetCarryFlag(full > 0xFF);

    a = result;
}

void SM83::scf() {
    LTRACE("SCF");

    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(true);
}

void SM83::set_dhl(u8 bit) {
    LTRACE("SET %u, (HL)", bit);
    u8 value = mmu.Read8(hl);

    value |= (1 << bit);
    mmu.Write8(hl, value);
}

void SM83::set(u8 bit, u8* reg) {
    *reg |= (1 << bit);
}

void SM83::sla_dhl() {
    LTRACE("SLA (HL)");

    u8 value = mmu.Read8(hl);
    bool should_carry = value & (1 << 7);
    u8 result = value << 1;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    mmu.Write8(hl, result);
}

void SM83::sla_r(u8* reg) {
    bool should_carry = *reg & (1 << 7);
    u8 result = *reg << 1;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(should_carry);

    *reg = result;
}

void SM83::sra_dhl() {
    LTRACE("SRA (HL)");

    u8 value = mmu.Read8(hl);
    u8 bit7 = value & (1 << 7);
    u8 result = value >> 1;

    SetZeroFlag((result | bit7) == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(value & 0x1);

    mmu.Write8(hl, result | bit7);
}

void SM83::sra_r(u8* reg) {
    u8 bit7 = *reg & (1 << 7);
    u8 result = *reg >> 1;

    SetZeroFlag((result | bit7) == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(*reg & 0x1);

    *reg = (result | bit7);
}

void SM83::srl_dhl() {
    LTRACE("SRL (HL)");

    u8 value = mmu.Read8(hl);
    u8 result = value >> 1;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(value & 0x1);

    mmu.Write8(hl, result);
}

void SM83::srl_r(u8* reg) {
    u8 result = *reg >> 1;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(*reg & 0x1);

    *reg = result;
}

void SM83::sub_r(u8 reg) {
    u8 result = a - reg;

    SetZeroFlag(a == reg);
    SetNegateFlag(true);
    SetHalfCarryFlag((a & 0xF) < (reg & 0xF));
    SetCarryFlag(a < reg);

    a = result;
}

void SM83::sub_d8() {
    u8 value = GetByteFromPC(); 
    LTRACE("SUB 0x%02X", value);

    u8 result = a - value;

    SetZeroFlag(a == value);
    SetNegateFlag(true);
    SetHalfCarryFlag((a & 0xF) < (value & 0xF));
    SetCarryFlag(a < value);

    a = result;
}

void SM83::xor_d8() {
    u8 value = GetByteFromPC();
    LTRACE("XOR 0x%02X", value);

    u8 result = a ^ value;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);

    a = result;
}

void SM83::xor_dhl() {
    LTRACE("XOR (HL)");

    u8 result = a ^ mmu.Read8(hl);

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);

    a = result;
}

void SM83::xor_r(u8 reg) {
    u8 result = a ^ reg;

    SetZeroFlag(result == 0);
    SetNegateFlag(false);
    SetHalfCarryFlag(false);
    SetCarryFlag(false);

    a = result;
}

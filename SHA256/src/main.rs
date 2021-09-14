/*
 * @Author: Yinwhe
 * @Date: 2021-09-13 10:56:07
 * @LastEditors: Yinwhe
 * @LastEditTime: 2021-09-14 13:38:42
 * @Description: SHA256 in rust
 * @Copyright: Copyright (c) 2021
 */
const K: [u32; 64] = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
];

struct Sha256 {
    pub state: [u32; 8],
    completed_data_blocks: u64,
    pending: [u8; 64],
    num_pending: usize,
}

impl Sha256 {
    pub fn default() -> Self {
        Self {
            state: [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19],
            completed_data_blocks: 0,
            pending: [0u8; 64],
            num_pending: 0,
        }
    }

    fn deal_one(&mut self, data: &[u8; 64]) {
        // println!("{:x?}", data);
        let mut w = [0u32; 64];
        let mut state = self.state.clone();
        for (i, w) in w[0..16].iter_mut().enumerate() {
            *w = u32::from_be_bytes(unsafe{*(data.as_ptr().add(4*i) as *const u8 as *const [u8; 4])});
        }

        for i in 16..64 {
            let s0 = w[i-15].rotate_right(7) ^ w[i-15].rotate_right(18) ^ (w[i-15] >> 3);
            let s1 = w[i-2].rotate_right(17) ^ w[i-2].rotate_right(19) ^ (w[i-2] >> 10);
            w[i] = w[i-16].wrapping_add(s0).wrapping_add(w[i-7]).wrapping_add(s1);
        }
        // println!("[w] - {:x?}", w);

        // println!("[state1] - {:x?}", state);
        for i in 0..64 {
            let ch = (state[4] & state[5]) ^ (!state[4] & state[6]);
            let maj = (state[0] & state[1]) ^ (state[0] & state[2]) ^ (state[1] & state[2]);
            let s0 = state[0].rotate_right(2) ^ state[0].rotate_right(13) ^ state[0].rotate_right(22);
            let s1 = state[4].rotate_right(6) ^ state[4].rotate_right(11) ^ state[4].rotate_right(25);
            let t0 = state[7].wrapping_add(s1).wrapping_add(ch).wrapping_add(K[i]).wrapping_add(w[i]);
            let t1 = s0.wrapping_add(maj);

            state[7] = state[6];
            state[6] = state[5];
            state[5] = state[4];
            state[4] = state[3].wrapping_add(t0);
            state[3] = state[2];
            state[2] = state[1];
            state[1] = state[0];
            state[0] = t0.wrapping_add(t1);
        }
        // println!("[state2] - {:x?}", state);

        for (i, v) in self.state.iter_mut().enumerate() {
            *v = v.wrapping_add(state[i]);
        }
        // println!("{:x?}", self.state);
    }

    fn deal_block(&mut self, data: &[u8]) {
        let mut offset = 0;
        let len = data.len();
        let mut block_num = len / 64;
        let mut remain_num = len % 64;

        if remain_num == 0 {
            block_num -= 1;
            remain_num += 64;
        }
        
        assert!((block_num as isize) >= 0);

        for _ in 0..block_num {
            self.deal_one(unsafe{&*(data.as_ptr().add(offset) as *const [u8; 64])});
            self.completed_data_blocks += 1;
            offset += 64;
        }

        self.num_pending = remain_num;
        self.pending[0..remain_num].copy_from_slice(&data[offset..]);
    }

    fn deal_remain(&mut self) -> [u8; 32] {
        let bits = self.completed_data_blocks * 512 + self.num_pending as u64 * 8;
        if self.num_pending < 56 {
            let mut block = self.pending.clone();
            block[self.num_pending] = 0x80;
            unsafe{
                block[56..].copy_from_slice(&bits.to_be_bytes() as &[u8; 8]);
            }
            self.deal_one(&block);
        } else {
            let mut blocks = [0u8; 128];
            blocks[..64].copy_from_slice(&self.pending);
            blocks[self.num_pending] = 0x80;
            unsafe{
                blocks[120..].copy_from_slice(&bits.to_be_bytes() as &[u8; 8]);
            }
            self.deal_one(unsafe {&*(blocks.as_ptr() as *const u8 as *const [u8; 64])});
            self.deal_one(unsafe {&*(blocks.as_ptr().add(64) as *const u8 as *const [u8; 64])});
        }
        for s in &mut self.state {
            * s = s.to_be();
        }
        unsafe { *(self.state.as_ptr() as *const [u8; 32]) }
    }

    pub fn digest(data: &[u8]) -> [u8; 32] {
        let mut sha = Sha256::default();
        sha.deal_block(data);
        sha.deal_remain()
    }
}

fn main() {
    let test = b"173af653133d964edfc16cafe0aba33c8f500a07f3ba3f81943916910c257705";
    let res = Sha256::digest(test);
    print!("0x");
    for x in res {
        print!("{:02x}", x);
    }
}

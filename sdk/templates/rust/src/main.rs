use std::io::{self, Read};

fn main() {
    let mut image = Vec::new();
    io::stdin().read_to_end(&mut image).expect("read PNG");
    println!(
        "{{\"protocol_version\":1,\"status\":\"success\",\"result\":{{\"type\":\"notification\",\"title\":\"@PLUGIN_NAME@\",\"text\":\"Received {} PNG bytes\"}}}}",
        image.len()
    );
}

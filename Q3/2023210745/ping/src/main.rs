// 导入所需的模块
use pnet::packet::icmp::{IcmpTypes, MutableIcmpPacket};
use pnet::packet::Packet;
use pnet::transport::{icmp_packet_iter, transport_channel, TransportChannelType::Layer4, TransportProtocol::Ipv4};
use std::env;
use std::net::{IpAddr, Ipv4Addr};
use std::process;
use std::str::FromStr;
use std::thread;
use std::time::{Duration, Instant};

// 定义ICMP包中的各个部分的长度
const ICMP_HEADER_LEN: usize = 8;  // ICMP头部长度
const ICMP_PAYLOAD_LEN: usize = 32; // ICMP负载长度
const ICMP_PACKET_LEN: usize = ICMP_HEADER_LEN + ICMP_PAYLOAD_LEN; // 完整的ICMP包长度

fn main() {
    // 获取命令行参数中的目标IP地址
    let target = env::args().nth(1).expect("Usage: ping <IP>");
    // 解析目标IP地址为IpAddr类型
    let target_addr = IpAddr::from_str(&target).expect("Invalid IP address.");

    // 如果目标地址是IPv4地址则调用ping函数；否则打印错误并退出
    if let IpAddr::V4(ipv4_addr) = target_addr {
        ping(ipv4_addr, 5);  // 执行ping操作，默认次数为5
    } else {
        eprintln!("IPv6 is not supported in this example.");
        process::exit(1); // 如果是IPv6地址，则退出程序
    }
}

// 定义ping函数，用于发送ICMP请求并接收响应
fn ping(destination: Ipv4Addr, count: usize) {
    // 设置协议为IPv4的ICMP
    let protocol = Layer4(Ipv4(pnet::packet::ip::IpNextHeaderProtocols::Icmp));
    // 创建传输通道以发送和接收ICMP消息
    let (mut sender, mut receiver) = transport_channel(1024, protocol)
        .expect("Error occurred when creating the transport channel");

    // 创建一个缓冲区以存储ICMP包
    let mut buffer = [0u8; ICMP_PACKET_LEN];
    // 初始化可变ICMP包
    let mut icmp_packet = MutableIcmpPacket::new(&mut buffer[..]).unwrap();
    // 设置ICMP类型为回显请求
    icmp_packet.set_icmp_type(IcmpTypes::EchoRequest);
    // 使用固定字节填充ICMP包的有效载荷
    icmp_packet.set_payload(&[0; ICMP_PAYLOAD_LEN]);
    // 计算校验和
    let checksum = pnet::util::checksum(icmp_packet.packet(), 1);
    // 设置ICMP包的校验和
    icmp_packet.set_checksum(checksum);

    // 打印ping操作的目标和数据字节数
    println!("PING {} ({}): {} bytes of data.", destination, destination, ICMP_PAYLOAD_LEN);

    // 循环指定的次数，发送ICMP请求
    for sequence in 1..=count {
        // 记录发送请求的时间点
        let start_time = Instant::now();

        // 发送ICMP回显请求
        sender.send_to(icmp_packet.to_immutable(), IpAddr::V4(destination))
            .expect("Failed to send ICMP packet");
        // 准备接收ICMP回显响应
        let mut iter = icmp_packet_iter(&mut receiver);
        
        // 等待并处理响应
        match iter.next_with_timeout(Duration::from_secs(1)) {
            // 如果接收到来自目标地址的包
            Ok(Some((packet, addr))) if addr == destination => {
                // 计算并打印往返时间
                let duration = start_time.elapsed();
                println!("{} bytes from {}: icmp_seq={} time={:?}",
                         packet.packet().len(),
                         addr,
                         sequence,  // 显示当前的序列号
                         duration);
            }
            // 忽略不来自目标地址的包
            Ok(Some((_packet, _addr))) => {}
            // 如果在1秒内没有响应，则认为请求超时
            Ok(None) => println!("Request timed out."),
            // 接收过程中出现错误
            Err(e) => {
                eprintln!("Error occurred when when receiving ICMP packet: {}", e);
                process::exit(1);  // 发生错误则退出程序
            }
        }
        // 两次ping操作之间等待1秒
        thread::sleep(Duration::from_secs(1)); 
    }
}

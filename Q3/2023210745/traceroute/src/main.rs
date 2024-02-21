// 引入pnet包中的相关模块，用于构造和解析ICMP数据包
use pnet::packet::icmp::{echo_request, IcmpTypes};
use pnet::packet::ip::IpNextHeaderProtocols;
use pnet::packet::Packet;
// 引入pnet包的传输通道功能，用于发送和接收数据包
use pnet::transport::{transport_channel, TransportChannelType, TransportProtocol};
// 引入标准库的环境模块、网络模块和时间模块
use std::env;
use std::net::{IpAddr, Ipv4Addr};
use std::time::Duration;

// 主函数，返回结果为泛型错误处理
fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 从命令行参数获取目的地地址
    let destination = env::args().nth(1).expect("请提供目标地址");
    // 将目的地址字符串解析为IPv4地址
    let destination: Ipv4Addr = destination.parse()?;
    // 设置传输协议为ICMP
    let protocol = TransportChannelType::Layer4(TransportProtocol::Ipv4(IpNextHeaderProtocols::Icmp));
    // 创建发送和接收通道
    let (mut sender, mut receiver) = transport_channel(1024, protocol)?;

    // 初始化序列号
    let mut seq_number = 0u16;
    // 设置时间生存值(TTL)范围为1到30，进行循环
    for ttl in 1..=30 {
        // 创建数据包缓冲区
        let mut packet = [0u8; 60];
        // 构建可变的ICMP回显请求数据包
        let mut icmp_packet = echo_request::MutableEchoRequestPacket::new(&mut packet[..]).unwrap();
        // 设置ICMP包的序列号
        icmp_packet.set_sequence_number(seq_number);
        // 设置ICMP类型为回显请求
        icmp_packet.set_icmp_type(IcmpTypes::EchoRequest);
        // 计算ICMP数据包的校验和
        let checksum = pnet::util::checksum(icmp_packet.packet(), 1);
        // 设置ICMP数据包的校验和
        icmp_packet.set_checksum(checksum);

        // 设置传输通道的TTL
        sender.set_ttl(ttl)?;
        // 发送ICMP包到指定的目标地址
        sender.send_to(icmp_packet, IpAddr::V4(destination))?;

        // 创建ICMP数据包迭代器用于接收响应
        let mut iter = pnet::transport::icmp_packet_iter(&mut receiver);
        // 记录当前时间，以计算往返时间
        let start = std::time::Instant::now();
        
        // 等待并获取ICMP响应，设置超时时间为1秒
        if let Some((_packet, addr)) = iter.next_with_timeout(Duration::from_secs(1))? {
            // 如果收到响应，计算持续时间
            let duration = start.elapsed();
            // 打印TTL、响应地址和时间
            println!("{}: {} time={:?}", ttl, addr, duration);
            // 如果响应地址是目标地址，终止循环
            if addr == IpAddr::V4(destination) { break; }
        } else {
            // 如果超时，则打印超时信息
            println!("{}: 请求超时", ttl);
        }
        
        // 序列号加1，为下一个数据包准备
        seq_number = seq_number.wrapping_add(1);
    }

    // 主函数正常结束
    Ok(())
}

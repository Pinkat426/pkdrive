#!/usr/bin/env python3
import subprocess
import re
import matplotlib.pyplot as plt
import os
from datetime import datetime

def get_memory_sections():
    """使用objdump获取内存段信息"""
    try:
        result = subprocess.run(['arm-none-eabi-objdump', '-h', 'build/Debug/pkdrive.elf'], 
                              capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        print(f"Error running objdump: {e}")
        return None

def get_memory_size():
    """使用size命令获取总体内存使用情况"""
    try:
        result = subprocess.run(['arm-none-eabi-size', 'build/Debug/pkdrive.elf'],
                              capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        print(f"Error running size: {e}")
        return None

def parse_sections(output):
    """解析objdump输出，提取段信息"""
    sections = []
    lines = output.split('\n')
    for line in lines:
        if any(x in line for x in ['.text', '.data', '.bss', '.rodata', '.ccmram']):
            parts = line.split()
            if len(parts) >= 4:
                sections.append({
                    'name': parts[1],
                    'size': int(parts[2], 16),
                    'vma': int(parts[3], 16)
                })
    return sections

def parse_size(output):
    """解析size命令输出"""
    lines = output.split('\n')
    if len(lines) < 2:
        return None
    
    parts = lines[1].split()
    if len(parts) >= 4:
        return {
            'text': int(parts[0]),
            'data': int(parts[1]),
            'bss': int(parts[2])
        }
    return None

def format_size(size_bytes):
    """格式化大小显示"""
    if size_bytes >= 1024 * 1024:
        return f'{size_bytes/1024/1024:.1f}MB'
    elif size_bytes >= 1024:
        return f'{size_bytes/1024:.1f}KB'
    else:
        return f'{size_bytes}B'

def create_memory_map(sections, size_info):
    """创建内存使用可视化图"""
    # 设置plt样式
    plt.style.use('default')  # 使用默认样式
    plt.rcParams['font.sans-serif'] = ['Microsoft YaHei']  # 使用微软雅黑字体
    plt.rcParams['axes.grid'] = True  # 启用网格
    plt.rcParams['grid.alpha'] = 0.3  # 设置网格透明度
    
    # 创建图形
    fig = plt.figure(figsize=(15, 10), facecolor='white')
    fig.suptitle('STM32F405RG 内存使用分析', fontsize=16, fontweight='bold', y=0.95)
    
    # 定义颜色映射
    colors = {
        '.text': '#2ecc71',     # 绿色
        '.rodata': '#3498db',   # 蓝色
        '.data': '#e74c3c',     # 红色
        '.bss': '#f1c40f',      # 黄色
        '.ccmram': '#9b59b6'    # 紫色
    }
    
    # Flash内存图 (左侧)
    ax1 = plt.subplot(1, 2, 1)
    flash_sections = [s for s in sections if s['vma'] >= 0x08000000]
    flash_total = 1024 * 1024  # 1MB Flash
    
    bottom = 0
    for section in flash_sections:
        size_kb = section['size'] / 1024
        color = colors.get(section['name'], '#95a5a6')
        plt.bar(0.5, size_kb, bottom=bottom, label=section['name'], 
                width=0.6, color=color, edgecolor='white', alpha=0.8)
        if size_kb > 5:
            plt.text(0.5, bottom + size_kb/2, 
                    f'{section["name"]}\n{format_size(section["size"])}', 
                    ha='center', va='center', fontweight='bold', color='white')
        bottom += size_kb
    
    flash_used = sum(s['size'] for s in flash_sections)
    flash_free = flash_total - flash_used
    plt.text(1.2, flash_total/2048, 
            f'Flash使用率:\n{flash_used/flash_total*100:.1f}%\n\n'
            f'已用: {format_size(flash_used)}\n'
            f'剩余: {format_size(flash_free)}', 
            bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
    
    plt.title('Flash内存 (1MB)', pad=20, fontsize=14, fontweight='bold')
    plt.ylim(0, flash_total/1024)
    plt.ylabel('大小 (KB)', fontsize=12)
    ax1.set_xticks([])
    ax1.grid(True, alpha=0.3)
    
    # RAM内存图 (右侧)
    ax2 = plt.subplot(1, 2, 2)
    ram_sections = [s for s in sections if 0x20000000 <= s['vma'] <= 0x20020000]
    ram_total = 128 * 1024  # 128KB RAM
    
    bottom = 0
    for section in ram_sections:
        size_kb = section['size'] / 1024
        color = colors.get(section['name'], '#95a5a6')
        plt.bar(0.5, size_kb, bottom=bottom, label=section['name'], 
                width=0.6, color=color, edgecolor='white', alpha=0.8)
        if size_kb > 5:
            plt.text(0.5, bottom + size_kb/2, 
                    f'{section["name"]}\n{format_size(section["size"])}', 
                    ha='center', va='center', fontweight='bold', color='white')
        bottom += size_kb
    
    ram_used = sum(s['size'] for s in ram_sections)
    ram_free = ram_total - ram_used
    plt.text(1.2, ram_total/2048, 
            f'RAM使用率:\n{ram_used/ram_total*100:.1f}%\n\n'
            f'已用: {format_size(ram_used)}\n'
            f'剩余: {format_size(ram_free)}', 
            bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
    
    plt.title('RAM内存 (128KB)', pad=20, fontsize=14, fontweight='bold')
    plt.ylim(0, ram_total/1024)
    plt.ylabel('大小 (KB)', fontsize=12)
    ax2.set_xticks([])
    ax2.grid(True, alpha=0.3)
    
    # 添加图例
    handles = [plt.Rectangle((0,0),1,1, color=color, alpha=0.8) 
              for color in colors.values()]
    labels = list(colors.keys())
    fig.legend(handles, labels, loc='center right', bbox_to_anchor=(0.98, 0.5))
    
    # 添加编译信息
    if size_info:
        info_text = (
            f'编译信息:\n'
            f'代码段(text): {format_size(size_info["text"])}\n'
            f'数据段(data): {format_size(size_info["data"])}\n'
            f'BSS段(bss): {format_size(size_info["bss"])}'
        )
        plt.figtext(0.02, 0.02, info_text, fontsize=10, 
                   bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
    
    # 添加时间戳
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    plt.figtext(0.98, 0.02, f'生成时间: {timestamp}', 
                fontsize=10, ha='right',
                bbox=dict(facecolor='white', alpha=0.8, edgecolor='none'))
    
    plt.tight_layout()
    
    # 保存图片
    os.makedirs('build/memory_analysis', exist_ok=True)
    plt.savefig('build/memory_analysis/memory_map.png', 
                dpi=300, bbox_inches='tight', 
                facecolor='white', edgecolor='none')
    plt.close()

def main():
    sections_output = get_memory_sections()
    size_output = get_memory_size()
    
    if not sections_output or not size_output:
        print("Failed to get memory information")
        return
    
    sections = parse_sections(sections_output)
    size_info = parse_size(size_output)
    
    if sections and size_info:
        create_memory_map(sections, size_info)
        print("Memory map generated: build/memory_analysis/memory_map.png")
    else:
        print("Failed to parse memory information")

if __name__ == '__main__':
    main() 
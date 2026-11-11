#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define HTML_FILE_PATH "/home/lukas/Documentos/TesteT1/index.html"

// Protótipos das funções
void get_version_info(char *buffer, size_t size);
void get_uptime(char *buffer, size_t size);
void get_idle_time(char *buffer, size_t size);
void get_system_time(char *buffer, size_t size);
void get_cpu_info(char *model_name, size_t size);
void get_system_load(char *buffer, size_t size);
void get_cpu_usage(char *buffer, size_t size);
void get_memory_usage(char *buffer, size_t size);
void get_io_operations(char *buffer, size_t size);
void get_supported_filesystems(char *buffer, size_t size);
void get_devices_and_groups(char *buffer, size_t size);
void get_network_info(char *buffer, size_t size);
void get_process_list(char *buffer, size_t size);
void generate_html();

int is_number(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

// Função auxiliar para ler arquivos do /proc
char* read_proc_file(const char* filename, char* buffer, size_t size) {
    memset(buffer, 0, size);
    FILE* file = fopen(filename, "r");
    if (!file) {
        snprintf(buffer, size, "Não foi possível abrir %s", filename);
        return NULL;
    }
    size_t bytesRead = fread(buffer, 1, size - 1, file);
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}

void get_version_info(char *buffer, size_t size) {
    memset(buffer, 0, size);
    read_proc_file("/proc/version", buffer, size);
}

void get_uptime(char *buffer, size_t size) {
    memset(buffer, 0, size);
    char uptime_str[256];
    if (read_proc_file("/proc/uptime", uptime_str, sizeof(uptime_str))) {
        double uptime_seconds;
        sscanf(uptime_str, "%lf", &uptime_seconds);
        int days = uptime_seconds / (24 * 3600);
        uptime_seconds -= days * 24 * 3600;
        int hours = uptime_seconds / 3600;
        uptime_seconds -= hours * 3600;
        int minutes = uptime_seconds / 60;
        int seconds = uptime_seconds - minutes * 60;
        snprintf(buffer, size, "<strong>Uptime:</strong> %d dias, %d horas, %d minutos, %d segundos\n", days, hours, minutes, seconds);
    }
}

void get_idle_time(char *buffer, size_t size) {
    FILE *file;
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    unsigned long idle_time = 0;

    file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice) == 10) {
        idle_time = idle;
    } else {
        perror("fscanf");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
    
    // Converte jiffies para segundos
    double idle_time_seconds = idle_time / (double)100;

    // Converte segundos para dias, horas, minutos e segundos
    unsigned long days = idle_time_seconds / 86400; // 1 dia = 86400 segundos
    unsigned long hours = (idle_time_seconds - (days * 86400)) / 3600; // 1 hora = 3600 segundos
    unsigned long minutes = (idle_time_seconds - (days * 86400) - (hours * 3600)) / 60; // 1 minuto = 60 segundos
    unsigned long seconds = idle_time_seconds - (days * 86400) - (hours * 3600) - (minutes * 60);


    snprintf(buffer, size, "<strong>Tempo Ocioso:</strong> %lu dias, %lu horas, %lu minutos, %lu segundos\n",
           days, hours, minutes, seconds);
}

void get_system_time(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *rtc = fopen("/proc/driver/rtc", "r");
    if (rtc == NULL) {
        snprintf(buffer, size, "Não foi possível abrir o arquivo /proc/driver/rtc.");
        return;
    }

    char line[256];
    char time_system[256] = {0};
    char date_system[256] = {0};

    while (fgets(line, sizeof(line), rtc)) {
        // Pegando o horário do sistema
        if (strncmp(line, "rtc_time", 8) == 0) {
            strncpy(time_system, strchr(line, ':') + 2, sizeof(time_system) - 1);
            time_system[strcspn(time_system, "\n")] = 0; // Remover nova linha
        }

        // Pegando a data do sistema
        if (strncmp(line, "rtc_date", 8) == 0) {
            strncpy(date_system, strchr(line, ':') + 2, sizeof(date_system) - 1);
            date_system[strcspn(date_system, "\n")] = 0; // Remover nova linha
        }
    }

    fclose(rtc);

    snprintf(buffer, size, "<strong>Horário  do Sistema:</strong> %s<br><strong>Data do Sistema:</strong> %s", time_system, date_system);
}

void get_cpu_info(char *buffer, size_t size) {
    FILE *file;
    char line[256];
    char model_name[256] = {0};
    char cpu_mhz[256] = {0};
    char cpu_cores[256] = {0};

    file = fopen("/proc/cpuinfo", "r");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *value = strchr(line, ':') + 2;
            if (value) {
                strncpy(model_name, value, sizeof(model_name) - 1);
                model_name[strcspn(model_name, "\n")] = 0; // Remover nova linha
            }
        }

        if (strncmp(line, "cpu MHz", 7) == 0) {
            char *value = strchr(line, ':') + 2;
            if (value) {
                strncpy(cpu_mhz, value, sizeof(cpu_mhz) - 1);
                cpu_mhz[strcspn(cpu_mhz, "\n")] = 0; // Remover nova linha
            }
        }

        if (strncmp(line, "cpu cores", 9) == 0) {
            char *value = strchr(line, ':') + 2;
            if (value) {
                strncpy(cpu_cores, value, sizeof(cpu_cores) - 1);
                cpu_cores[strcspn(cpu_cores, "\n")] = 0; // Remover nova linha
            }
        }
    }

    fclose(file);

    // Format the output as HTML
    snprintf(buffer, size,
        "<strong>Modelo do Processador:</strong> %s<br>"
        "<strong>CPU MHz:</strong> %s<br>"
        "<strong>Número de Núcleos:</strong> %s<br>",
        model_name, cpu_mhz, cpu_cores);
}

void get_system_load(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *systemload = fopen("/proc/loadavg", "r");
    if (systemload) {
        double x1, x2, x3;
        unsigned int running_processes, total_processes, last_pid;

        if (fscanf(systemload, "%lf %lf %lf %u/%u %u", &x1, &x2, &x3, &running_processes, &total_processes, &last_pid) == 6) {
            snprintf(buffer, size,
                "<strong>Carga média nos últimos 1 minuto:</strong> %.2f<br>"
                "<strong>Carga média nos últimos 5 minutos:</strong> %.2f<br>"
                "<strong>Carga média nos últimos 15 minutos:</strong> %.2f<br>"
                "<strong>Processos em Execução/Total:</strong> %u/%u<br>"
                "<strong>Último PID:</strong> %u",
                x1, x2, x3, running_processes, total_processes, last_pid);
        } else {
            snprintf(buffer, size, "Erro ao ler /proc/loadavg");
        }
        fclose(systemload);
    } else {
        snprintf(buffer, size, "Não foi possível abrir /proc/loadavg");
    }
}

void get_cpu_usage(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *statFP;
    int oldNumbers[7], newNumbers[7];
    int diffNumbers[7];
    char cpu[10];  // Não usado, mas necessário para leitura

    statFP = fopen("/proc/stat", "r");
    if (statFP == NULL) {
        snprintf(buffer, size, "Não foi possível abrir /proc/stat.");
        return;
    }

    // Leitura das contagens antigas
    fscanf(statFP,
            "%s %d %d %d %d %d %d %d",
            cpu,
            &oldNumbers[0], 
            &oldNumbers[1],
            &oldNumbers[2],
            &oldNumbers[3],
            &oldNumbers[4],
            &oldNumbers[5],
            &oldNumbers[6]);

    sleep(1); // Espera 1 segundo para medir a diferença
    rewind(statFP);

    // Leitura das contagens novas
    fscanf(statFP,
            "%s %d %d %d %d %d %d %d",
            cpu,
            &newNumbers[0],
            &newNumbers[1],
            &newNumbers[2],
            &newNumbers[3],
            &newNumbers[4],
            &newNumbers[5],
            &newNumbers[6]);

    fclose(statFP);

    // Cálculo das diferenças
    for (int ii = 0; ii < 7; ii++) {
        diffNumbers[ii] = newNumbers[ii] - oldNumbers[ii];
    }

    // Cálculo do uso do CPU
    int total_old = oldNumbers[0] + oldNumbers[1] + oldNumbers[2] + oldNumbers[3];
    int total_new = newNumbers[0] + newNumbers[1] + newNumbers[2] + newNumbers[3];
    int total_diff = total_new - total_old;
    int idle_diff = diffNumbers[3]; // Idle é o quarto valor

    float usage_percent = 100.0 * (total_diff - idle_diff) / total_diff;

    snprintf(buffer, size, "<strong>Uso do Processador:</strong> %.2f%%", usage_percent);
}

void get_memory_usage(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *memoryusage = fopen("/proc/meminfo", "r");
    if (memoryusage == NULL) {
        snprintf(buffer, size, "Não foi possível abrir o arquivo /proc/meminfo.");
        return;
    }

    char line[256];
    double mem_total = 0.0;
    double mem_available = 0.0;

    while (fgets(line, sizeof(line), memoryusage)) {
        // Pegando o valor de MemTotal
        if (strncmp(line, "MemTotal", 8) == 0) {
            sscanf(strchr(line, ':') + 1, "%lf", &mem_total);
        }

        // Pegando o valor de MemAvailable
        if (strncmp(line, "MemAvailable", 12) == 0) {
            sscanf(strchr(line, ':') + 1, "%lf", &mem_available);
        }
    }

    fclose(memoryusage);

    snprintf(buffer, size, "<strong>Memória Total:</strong> %.2f mb<br><strong>Memória Disponível:</strong> %.2f mb", mem_total/1024, mem_available/1024);
}

void get_io_operations(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *diskstats = fopen("/proc/diskstats", "r");
    if (diskstats == NULL) {
        snprintf(buffer, size, "Não foi possível abrir o arquivo /proc/diskstats.");
        return;
    }

    memset(buffer, 0, size);
    char line[256];
    size_t used_size = 0;  // Tamanho do buffer já usado

    while (fgets(line, sizeof(line), diskstats)) {
        unsigned long long reads, reads_merged, sectors_read, time_reading;
        unsigned long long writes, writes_merged, sectors_written, time_writing;
        char device[64];

        // A linha é dividida em campos separados por espaço
        if (sscanf(line, "%*d %*d %63s %llu %llu %llu %llu %llu %llu %llu %llu",
                   device,
                   &reads,
                   &reads_merged,
                   &sectors_read,
                   &time_reading,
                   &writes,
                   &writes_merged,
                   &sectors_written,
                   &time_writing) == 9) {

            // Verifique se há espaço restante no buffer
            int remaining_size = size - used_size;
            if (remaining_size <= 0) {
                break;  // Buffer cheio, pare de adicionar dados
            }

            // Adicione as informações ao buffer
            int written = snprintf(buffer + used_size, remaining_size,
                    "<strong>Dispositivo:</strong> %s<br>"
                    "<strong>Leituras:</strong> %llu<br>"
                    "<strong>Leituras Mescladas:</strong> %llu<br>"
                    "<strong>Setores Lidos:</strong> %llu<br>"
                    "<strong>Tempo Lendo (ms):</strong> %llu<br>"
                    "<strong>Escritas:</strong> %llu<br>"
                    "<strong>Escritas Mescladas:</strong> %llu<br>"
                    "<strong>Setores Escritos:</strong> %llu<br>"
                    "<strong>Tempo Escrevendo (ms):</strong> %llu<br><br>",
                    device, reads, reads_merged, sectors_read, time_reading,
                    writes, writes_merged, sectors_written, time_writing);

            // Verifique se o snprintf foi bem-sucedido
            if (written < 0 || written >= remaining_size) {
                // Se houver erro ou não houver espaço suficiente, pare
                break;
            }

            // Atualiza o tamanho já usado no buffer
            used_size += written;
        }
    }

    fclose(diskstats);
}

void get_supported_filesystems(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *filesystems = fopen("/proc/filesystems", "r");
    if (filesystems == NULL) {
        snprintf(buffer, size, "Não foi possível abrir o arquivo /proc/filesystems.");
        return;
    }

    char line[256];
    char filesystem[64];
    int first = 1; // Para controlar a primeira entrada e evitar a vírgula inicial
    size_t offset = 0; // Offset no buffer

    // Inicia o buffer com o título
    offset += snprintf(buffer, size, "<strong>Sistema de Arquivos Suportados:</strong> [");

    while (fgets(line, sizeof(line), filesystems)) {
        // Os sistemas de arquivos suportados podem ter um prefixo "nodev"
        if (sscanf(line, "%*s %63s", filesystem) == 1 || sscanf(line, "%63s", filesystem) == 1) {
            if (!first) {
                offset += snprintf(buffer + offset, size - offset, ", ");
            }
            offset += snprintf(buffer + offset, size - offset, "%s", filesystem);
            first = 0; // Após a primeira inserção, marcar que já há entrada
        }
    }

    // Fechar a lista com o colchete de fechamento
    snprintf(buffer + offset, size - offset, "]");

    fclose(filesystems);
}

void get_devices_and_groups(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *devices = fopen("/proc/devices", "r");
    if (devices == NULL) {
        snprintf(buffer, size, "Não foi possível abrir o arquivo /proc/devices.");
        return;
    }

    char line[size];
    int in_character_devices_section = 0;
    int in_block_devices_section = 0;
    
    while (fgets(line, sizeof(line), devices)) {
        // Verifica se a linha é o início da seção de dispositivos de caractere
        if (strncmp(line, "Character devices:", 18) == 0) {
            if (in_block_devices_section) {
                // Adiciona o título "Block Devices" e o conteúdo da seção de dispositivos de bloco ao buffer
                strcat(buffer, "<br><strong>Block Devices:</strong>");
                in_block_devices_section = 0;
            }
            in_character_devices_section = 1;
            strcat(buffer, "<strong>Character Devices:</strong>");
            continue;
        }

        // Verifica se a linha é o início da seção de dispositivos de bloco
        if (strncmp(line, "Block devices:", 14) == 0) {
            if (in_character_devices_section) {
                // Adiciona o título "Character Devices" e o conteúdo da seção de dispositivos de caractere ao buffer
                strcat(buffer, "<br><strong>Block Devices:</strong>");
                in_character_devices_section = 0;
            }
            in_block_devices_section = 1;
            continue;
        }

        // Se estiver na seção de dispositivos de caractere, armazena no buffer
        if (in_character_devices_section) {
            strcat(buffer, line);
        }

        // Se estiver na seção de dispositivos de bloco, armazena no buffer
        if (in_block_devices_section) {
            strcat(buffer, line);
        }
    }

    fclose(devices);
    
}

void get_network_info(char *buffer, size_t size) {
    memset(buffer, 0, size);
    FILE *netdev = fopen("/proc/net/dev", "r");
    if (netdev == NULL) {
        snprintf(buffer, size, "Não foi possível abrir o arquivo /proc/net/dev.");
        return;
    }

    char line[256];
    memset(buffer, 0, size);
    
    // Pular o cabeçalho
    fgets(line, sizeof(line), netdev);
    fgets(line, sizeof(line), netdev);

    while (fgets(line, sizeof(line), netdev)) {
        char iface[64];
        unsigned long long recv_bytes, recv_packets, recv_errs, recv_drop;
        unsigned long long send_bytes, send_packets, send_errs, send_drop;

        // Lê os dados da linha
        if (sscanf(line, "%[^:]: %llu %llu %llu %llu %*s %*s %*s %*s %llu %llu %llu %llu",
                   iface,
                   &recv_bytes,
                   &recv_packets,
                   &recv_errs,
                   &recv_drop,
                   &send_bytes,
                   &send_packets,
                   &send_errs,
                   &send_drop) == 9) {
            snprintf(buffer + strlen(buffer), size - strlen(buffer),
                     "<br><strong>Interface:</strong> %s<br>"
                     "<strong>Bytes Recebidos:</strong> %llu<br>"
                     "<strong>Pacotes Recebidos:</strong> %llu<br>"
                     "<strong>Erros Recebidos:</strong> %llu<br>"
                     "<strong>Pacotes Perdidos Recebidos:</strong> %llu<br>"
                     "<strong>Bytes Enviados:</strong> %llu<br>"
                     "<strong>Pacotes Enviados:</strong> %llu<br>"
                     "<strong>Erros Enviados:</strong> %llu<br>"
                     "<strong>Pacotes Perdidos Enviados:</strong> %llu<br>",
                     iface, recv_bytes, recv_packets, recv_errs, recv_drop,
                     send_bytes, send_packets, send_errs, send_drop);
        }
    }

    fclose(netdev);
}

void get_process_list(char *buffer, size_t size) {
    DIR *dp;
    struct dirent *entry;
    char path[BUFFER_SIZE];
    char process_name[BUFFER_SIZE];
    size_t pos = 0;

    dp = opendir("/proc");
    if (dp == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Iterate through directory entries
    while ((entry = readdir(dp)) != NULL) {
        // Check if the directory name is a number
        if (entry->d_type == DT_DIR && is_number(entry->d_name)) {
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

            FILE *file = fopen(path, "r");
            if (file) {
                if (fgets(process_name, sizeof(process_name), file) != NULL) {
                    process_name[strcspn(process_name, "\n")] = '\0'; // Remove newline

                    // Append process information to buffer
                    pos += snprintf(buffer + pos, size - pos,
                        "<strong>%s:</strong> %s<br>\n", entry->d_name, process_name);

                    // Check if buffer is almost full
                    if (size - pos < 100) {
                        break;
                    }
                }
                fclose(file);
            }
        }
    }

    closedir(dp);
}

// Função para gerar o conteúdo HTML
void generate_html() {
    FILE *htmlFile = fopen(HTML_FILE_PATH, "w");
    if (!htmlFile) {
        perror("Não foi possível abrir o arquivo HTML");
        return;
    }

    int fd = fileno(htmlFile);
    if (flock(fd, LOCK_EX) == -1) {
        perror("Erro ao bloquear o arquivo");
        fclose(htmlFile);
        return;
    }

    char buffer[BUFFER_SIZE];
    
    fprintf(htmlFile, "<!DOCTYPE html>\n<html lang=\"pt-BR\">\n<head>\n<meta charset=\"UTF-8\">\n"
                      "<title>Informações do Sistema</title>\n</head>\n<body>\n");

    #define WRITE_INFO(func) \
        func(buffer, sizeof(buffer)); \
        fprintf(htmlFile, "<p>%s</p>\n", buffer);

    WRITE_INFO(get_version_info)
    WRITE_INFO(get_uptime)
    WRITE_INFO(get_system_time)
    WRITE_INFO(get_idle_time)
    WRITE_INFO(get_cpu_info)
    WRITE_INFO(get_system_load)
    WRITE_INFO(get_cpu_usage)
    WRITE_INFO(get_memory_usage)
    WRITE_INFO(get_io_operations)
    WRITE_INFO(get_supported_filesystems)
    WRITE_INFO(get_devices_and_groups)
    WRITE_INFO(get_network_info)
    WRITE_INFO(get_process_list)

    fprintf(htmlFile, "</body>\n</html>\n");

    flock(fd, LOCK_UN);
    fclose(htmlFile);
}

int main() {
    while (1) {
        generate_html();
        sleep(4);
    }
    return 0;
}
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <grp.h>
#include <stdlib.h>
#include <pwd.h>
#include <assert.h>

struct Node {
	char *name;
	mode_t type;
	char *last_modification;
	char *group;
	char *user;
	int size;
	struct Node *next;
};
typedef struct Node Node;

int n_digits(int n){
	int count = 0;
	while (n / 10 > 0){
		n = n / 10;
		count++;
	}
	count++;

	return count;
}

void insert(Node *node, char *name, mode_t type, char *last_modification, char *group, char *user, int size){
	Node *n = node;
	assert(n != NULL);

	while (n->next != NULL){
		n = n->next;
	}
	
	n->next = (Node *)malloc(sizeof(Node));
	assert(n->next != NULL);
	n->next->next = NULL;

	n->name = (char *)malloc(strlen(name));
	strcpy(n->name, name);
	n->type = type;
	n->last_modification = (char *)malloc(strlen(last_modification));
	strcpy(n->last_modification, last_modification);
	n->group = (char *)malloc(strlen(group));
	strcpy(n->group, group);
	n->user = (char *)malloc(strlen(user));
	strcpy(n->user, user);
	n->size = size;
}

int main(int argc, char **argv){ 
	char path[255];
	if (argc < 2){
		strcpy(path, "./");	
	} else {
		strcpy(path, argv[1]);
		int length = strlen(path);
		if (path[length - 1] != '/') {
			path[length] = '/';
			path[length + 1] = '\0';
		}
	}

	struct dirent *entry;
	DIR *dp = opendir(path);
	assert(dp != NULL);

	Node *head;
	head = (Node *)malloc(sizeof(Node));
	assert(head != NULL);

	struct stat entry_stat_info;
	struct group *entry_group_info;
	struct passwd *entry_user_info;
	struct tm *entry_time_info;

	char full_path[255];
	char timestamp[15];
	char months[12][4] = {"Jan\0", "Feb\0", "Mar\0", "Apr\0", "May\0", "Jun\0", "Jul\0", "Aug\0", "Sep\0", "Oct\0", "Nov\0", "Dec\0"};
	while((entry = readdir(dp))){
		sprintf(full_path, "%s%s", path, entry->d_name);
		stat(full_path, &entry_stat_info);

		entry_user_info = getpwuid(entry_stat_info.st_uid);
		entry_group_info = getgrgid(entry_stat_info.st_gid);
		entry_time_info = localtime(&(entry_stat_info.st_mtime));

		sprintf(timestamp, "%s %02d %02d:%02d", months[entry_time_info->tm_mon], entry_time_info->tm_mday, entry_time_info->tm_hour, entry_time_info->tm_min);
		
		insert(head, entry->d_name, entry_stat_info.st_mode, timestamp, entry_group_info->gr_name, entry_user_info->pw_name, entry_stat_info.st_size);
	}

	closedir(dp);

	int group_width = 0, user_width = 0, size_width = 0;

	Node *n = head;

	while (n->next != NULL){
		int group_length = strlen(n->group);
		int user_length = strlen(n->user);
		int size_length = n_digits(n->size);
		if (group_length > group_width) group_width = group_length;
		if (user_length > user_width) user_width = user_length;
		if (size_length > size_width) size_width = size_length;

		n = n->next;
	}

	Node *n2 = head;

	while (n2->next != NULL){
		printf("%c", n2->type & S_IFDIR ? 'd' : '-');
		printf("%c%c%c", n2->type & S_IRUSR ? 'r' : '-', n2->type & S_IWUSR ? 'w' : '-', n2->type & S_IXUSR ? 'x' : '-');
		printf("%c%c%c", n2->type & S_IRGRP ? 'r' : '-', n2->type & S_IWGRP ? 'w' : '-', n2->type & S_IXGRP ? 'x' : '-');
		printf("%c%c%c ", n2->type & S_IROTH ? 'r' : '-', n2->type & S_IWOTH ? 'w' : '-', n2->type & S_IXOTH ? 'x' : '-');
		printf("%-*s %-*s %*d %s %s\n", group_width, n2->group, user_width, n2->user, size_width, n2->size, n2->last_modification, n2->name);

		n2 = n2->next;
	}

	return 0;
}
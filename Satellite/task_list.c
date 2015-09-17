#include <c.h>

#include "desk.h"
#include "report.h"
#include "task_list.h"
#include "tasks.h"

void **
initTaskList(struct desk *desk)
{
    long long	    totalListSize;
	size_t		    maxListPageSize;
    int			    numberOfPages;
    size_t		    eachPageSize;
    size_t		    lastPageSize;
    int			    tasksPerPage;
    int				pageId;
    int				taskId;
    int				taskIdInPage;
    size_t			pageSize;
	void			**list;
	void			*page;
	struct task		**taskInList;

	maxListPageSize = MAX_TASK_LIST_PAGE_SIZE;

	tasksPerPage = MAX_TASK_LIST_PAGE_SIZE / sizeof(struct task *);

	totalListSize = (long)MAX_NUMBER_OF_TASK * (long)sizeof(struct task *);
	if (totalListSize <= maxListPageSize) {
		numberOfPages = 1;
		eachPageSize = totalListSize;
		lastPageSize = 0;
	} else {
		if ((totalListSize % maxListPageSize) == 0) {
			numberOfPages = totalListSize / maxListPageSize;
			eachPageSize = maxListPageSize;
			lastPageSize = 0;
		} else {
			numberOfPages = (totalListSize / maxListPageSize) + 1;
			eachPageSize = maxListPageSize;
			lastPageSize = totalListSize % maxListPageSize;
		}
	}

	list = malloc(numberOfPages * sizeof(struct task *));
	if (list == NULL) {
        reportError("Out of memory");
        return NULL;
    }

	pageId = 0;
	taskId = 0;
	taskIdInPage = tasksPerPage;

	while (taskId < MAX_NUMBER_OF_TASK)
	{
		if (taskIdInPage == tasksPerPage) {
			taskIdInPage = 0;

			if ((taskId + 1) < numberOfPages) {
				pageSize = eachPageSize;
			} else {
				pageSize = (lastPageSize == 0) ? eachPageSize : lastPageSize;
			}

	    	page = malloc(pageSize);
			if (page == NULL) {
        		reportError("Out of memory");
    	    	return NULL;
		    }

			list[pageId] = page;
			pageId++;
		}

		taskInList = (void *)((unsigned long)page + (unsigned long)(taskIdInPage * sizeof(struct task *)));
		*taskInList = NULL;

		taskIdInPage++;
		taskId++;
	}

	return list;
}

void
taskListPushTask(struct desk *desk, int taskId, struct task *task)
{
	size_t		    maxListPageSize;
    int			    tasksPerPage;
    int				pageId;
    int				taskIdInPage;
	void			*page;
	struct task		**taskInList;

	maxListPageSize = MAX_TASK_LIST_PAGE_SIZE;

	tasksPerPage = MAX_TASK_LIST_PAGE_SIZE / sizeof(struct task *);

	pageId = taskId / tasksPerPage;
	taskIdInPage = taskId % tasksPerPage;
	page = desk->tasks.list[pageId];

	taskInList = (void *)((unsigned long)page + (unsigned long)(taskIdInPage * sizeof(struct task *)));
	*taskInList = task;
}

struct task *
taskListTaskById(struct desk *desk, int taskId)
{
	size_t		    maxListPageSize;
    int			    tasksPerPage;
    int				pageId;
    int				taskIdInPage;
	void			*page;
	struct task		**taskInList;

	maxListPageSize = MAX_TASK_LIST_PAGE_SIZE;

	tasksPerPage = MAX_TASK_LIST_PAGE_SIZE / sizeof(struct task *);

	pageId = taskId / tasksPerPage;
	taskIdInPage = taskId % tasksPerPage;
	page = desk->tasks.list[pageId];

	taskInList = (void *)((unsigned long)page + (unsigned long)(taskIdInPage * sizeof(struct task *)));

	return *taskInList;
}
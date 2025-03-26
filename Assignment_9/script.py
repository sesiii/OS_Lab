# import os
# import subprocess
# import time
# from datetime import datetime

# def get_untracked_files():
#     """ Get a list of untracked (new) files """
#     result = subprocess.run(["git", "ls-files", "--others", "--exclude-standard"], capture_output=True, text=True)
#     return result.stdout.splitlines()

# def get_modified_files():
#     """ Get a list of modified files """
#     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
#     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" M ")]

# def get_deleted_files():
#     """ Get a list of deleted files """
#     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
#     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" D ")]

# def commit_and_push():
#     modified_files = get_modified_files()
#     untracked_files = get_untracked_files()
#     deleted_files = get_deleted_files()
#     all_files = modified_files + untracked_files + deleted_files

#     if not all_files:
#         print("No new, modified, or deleted files to commit.")
#         return

#     for file in all_files:
#         if file in deleted_files:
#             subprocess.run(["git", "rm", file])  # Remove deleted files
#         else:
#             subprocess.run(["git", "add", file])  # Add new/modified files

#         current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
#         commit_message = f"changes in {os.path.basename(file)} on {current_time}"
#         subprocess.run(["git", "commit", "-m", commit_message])

#         # Push immediately after each commit to increase contribution count
#         subprocess.run(["git", "push", "origin", "main"])  # Change branch if necessary
#         time.sleep(2)  # Small delay to mimic manual commits

#     print("All changes committed and pushed successfully!")

# if __name__ == "__main__":
#     commit_and_push()


import os
import subprocess
import time
import random
from datetime import datetime

def get_untracked_files():
    """ Get a list of untracked (new) files """
    result = subprocess.run(["git", "ls-files", "--others", "--exclude-standard"], capture_output=True, text=True)
    return result.stdout.splitlines()

def get_modified_files():
    """ Get a list of modified files """
    result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
    return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" M ")]

def get_deleted_files():
    """ Get a list of deleted files """
    result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
    return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" D ")]

def generate_commit_message(filename):
    """ Generate a dynamic and realistic commit message """
    templates = [
        "Updated {file} with new changes",
        "Refactored {file} for better performance",
        "Fixed minor issues in {file}",
        "Added new content to {file}",
        "Cleaned up {file} and optimized code",
        "Improved readability of {file}",
        "Resolved conflicts in {file}",
        "Enhanced {file} with additional functionality",
        "Removed deprecated sections from {file}",
        "General improvements to {file}",
    ]
    
    commit_message = random.choice(templates).format(file=os.path.basename(filename))
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    return f"{commit_message} on {timestamp}"

def commit_and_push():
    modified_files = get_modified_files()
    untracked_files = get_untracked_files()
    deleted_files = get_deleted_files()
    all_files = modified_files + untracked_files + deleted_files

    if not all_files:
        print("No new, modified, or deleted files to commit.")
        return

    for file in all_files:
        if file in deleted_files:
            subprocess.run(["git", "rm", file])  # Remove deleted files
        else:
            subprocess.run(["git", "add", file])  # Add new/modified files

        commit_message = generate_commit_message(file)
        subprocess.run(["git", "commit", "-m", commit_message])

        # Push immediately after each commit
        subprocess.run(["git", "push", "origin", "main"])  # Change branch if necessary
        time.sleep(2)  # Small delay to mimic manual commits

    print("All changes committed and pushed successfully!")

if __name__ == "__main__":
    commit_and_push()
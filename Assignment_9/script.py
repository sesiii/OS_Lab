# # # import os
# # # import subprocess
# # # import time
# # # from datetime import datetime

# # # def get_untracked_files():
# # #     """ Get a list of untracked (new) files """
# # #     result = subprocess.run(["git", "ls-files", "--others", "--exclude-standard"], capture_output=True, text=True)
# # #     return result.stdout.splitlines()

# # # def get_modified_files():
# # #     """ Get a list of modified files """
# # #     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
# # #     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" M ")]

# # # def get_deleted_files():
# # #     """ Get a list of deleted files """
# # #     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
# # #     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" D ")]

# # # def commit_and_push():
# # #     modified_files = get_modified_files()
# # #     untracked_files = get_untracked_files()
# # #     deleted_files = get_deleted_files()
# # #     all_files = modified_files + untracked_files + deleted_files

# # #     if not all_files:
# # #         print("No new, modified, or deleted files to commit.")
# # #         return

# # #     for file in all_files:
# # #         if file in deleted_files:
# # #             subprocess.run(["git", "rm", file])  # Remove deleted files
# # #         else:
# # #             subprocess.run(["git", "add", file])  # Add new/modified files

# # #         current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
# # #         commit_message = f"changes in {os.path.basename(file)} on {current_time}"
# # #         subprocess.run(["git", "commit", "-m", commit_message])

# # #         # Push immediately after each commit to increase contribution count
# # #         subprocess.run(["git", "push", "origin", "main"])  # Change branch if necessary
# # #         time.sleep(2)  # Small delay to mimic manual commits

# # #     print("All changes committed and pushed successfully!")

# # # if __name__ == "__main__":
# # #     commit_and_push()


# # import os
# # import subprocess
# # import time
# # import random
# # from datetime import datetime

# # def get_untracked_files():
# #     """ Get a list of untracked (new) files """
# #     result = subprocess.run(["git", "ls-files", "--others", "--exclude-standard"], capture_output=True, text=True)
# #     return result.stdout.splitlines()

# # def get_modified_files():
# #     """ Get a list of modified files """
# #     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
# #     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" M ")]

# # def get_deleted_files():
# #     """ Get a list of deleted files """
# #     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
# #     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" D ")]

# # def generate_commit_message(filename):
# #     """ Generate a dynamic and realistic commit message """
# #     templates = [
# #         "Updated {file} with new changes",
# #         "Refactored {file} for better performance",
# #         "Fixed minor issues in {file}",
# #         "Added new content to {file}",
# #         "Cleaned up {file} and optimized code",
# #         "Improved readability of {file}",
# #         "Resolved conflicts in {file}",
# #         "Enhanced {file} with additional functionality",
# #         "Removed deprecated sections from {file}",
# #         "General improvements to {file}",
# #     ]
    
# #     commit_message = random.choice(templates).format(file=os.path.basename(filename))
# #     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
# #     return f"{commit_message} on {timestamp}"

# # def commit_and_push():
# #     modified_files = get_modified_files()
# #     untracked_files = get_untracked_files()
# #     deleted_files = get_deleted_files()
# #     all_files = modified_files + untracked_files + deleted_files

# #     if not all_files:
# #         print("No new, modified, or deleted files to commit.")
# #         return

# #     for file in all_files:
# #         if file in deleted_files:
# #             subprocess.run(["git", "rm", file])  # Remove deleted files
# #         else:
# #             subprocess.run(["git", "add", file])  # Add new/modified files

# #         commit_message = generate_commit_message(file)
# #         subprocess.run(["git", "commit", "-m", commit_message])

# #         # Push immediately after each commit
# #         subprocess.run(["git", "push", "origin", "main"])  # Change branch if necessary
# #         time.sleep(2)  # Small delay to mimic manual commits

# #     print("All changes committed and pushed successfully!")

# # if __name__ == "__main__":
# #     commit_and_push()



# import os
# import subprocess
# import time
# import random
# import requests
# from datetime import datetime

# # Together AI API configuration
# TOGETHER_API_KEY = os.getenv("TOGETHER_API_KEY")  # Set this in your environment
# API_URL = "https://api.together.xyz/v1/completions"
# MODEL = "meta-llama/Llama-3.1-8B-Instruct"

# def get_untracked_files():
#     """Get a list of untracked (new) files"""
#     result = subprocess.run(["git", "ls-files", "--others", "--exclude-standard"], capture_output=True, text=True)
#     return result.stdout.splitlines()

# def get_modified_files():
#     """Get a list of modified files"""
#     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
#     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" M ")]

# def get_deleted_files():
#     """Get a list of deleted files"""
#     result = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
#     return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" D ")]

# def get_file_diff(filename):
#     """Get the git diff for a modified file"""
#     result = subprocess.run(["git", "diff", "--cached", "--", filename], capture_output=True, text=True)
#     if not result.stdout:  # If not staged, get the unstaged diff
#         result = subprocess.run(["git", "diff", "--", filename], capture_output=True, text=True)
#     return result.stdout

# def call_together_api(diff):
#     """Call Together AI API to generate a commit message based on the diff"""
#     if not TOGETHER_API_KEY:
#         raise ValueError("TOGETHER_API_KEY environment variable not set")

#     prompt = (
#         "You are a senior developer writing a concise, realistic Git commit message. "
#         "Based on the following git diff, write a detailed and professional commit message "
#         "describing the changes. Include specifics about what was added, removed, or modified, "
#         "and why. Keep it under 80 characters if possible:\n\n"
#         f"```diff\n{diff}\n```"
#     )

#     headers = {
#         "Authorization": f"Bearer {TOGETHER_API_KEY}",
#         "Content-Type": "application/json",
#     }
#     payload = {
#         "model": MODEL,
#         "prompt": prompt,
#         "max_tokens": 100,
#         "temperature": 0.7,
#         "top_p": 0.9,
#     }

#     response = requests.post(API_URL, json=payload, headers=headers)
#     response.raise_for_status()
#     return response.json()["choices"][0]["text"].strip()

# def generate_fallback_message(filename, is_deleted=False):
#     """Generate a fallback commit message for new or deleted files"""
#     file_name = os.path.basename(filename)
#     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
#     if is_deleted:
#         return f"Removed {file_name} - obsolete as of {timestamp}"
    
#     file_ext = os.path.splitext(filename)[1].lower()
#     if file_ext in [".py", ".js"]:
#         return f"Added {file_name} with initial implementation ({timestamp})"
#     elif file_ext in [".md", ".txt"]:
#         return f"Created {file_name} with initial draft ({timestamp})"
#     else:
#         return f"Introduced {file_name} to the project ({timestamp})"

# def generate_commit_message(filename, is_deleted=False):
#     """Generate a commit message, using LLM for modified files or fallback for others"""
#     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
#     if is_deleted:
#         return generate_fallback_message(filename, is_deleted=True)
    
#     if filename in get_modified_files():
#         diff = get_file_diff(filename)
#         if diff:
#             try:
#                 message = call_together_api(diff)
#                 return f"{message} ({timestamp})"
#             except Exception as e:
#                 print(f"Failed to get LLM message for {filename}: {e}")
#                 return f"Updated {os.path.basename(filename)} with recent changes ({timestamp})"
    
#     # Fallback for untracked (new) files
#     return generate_fallback_message(filename)

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
#             commit_message = generate_commit_message(file, is_deleted=True)
#         else:
#             subprocess.run(["git", "add", file])  # Add new/modified files
#             commit_message = generate_commit_message(file)

#         subprocess.run(["git", "commit", "-m", commit_message])

#         # Push immediately after each commit
#         subprocess.run(["git", "push", "origin", "main"])  # Change branch if necessary
#         time.sleep(2)  # Small delay to mimic manual commits

#     print("All changes committed and pushed successfully!")

# if __name__ == "__main__":
#     commit_and_push()


import os
import subprocess
import time
from datetime import datetime
from together import Together

# Hardcoded Together AI API key
API_KEY = "b1f8ad7fd5671241f7901997ee157ec8fb7475ba8cbf6a85118a06d1f3de7fd5"
client = Together(api_key=API_KEY)

# Specify the directory for Git operations (change this as needed)
DIRECTORY = "/home/dadi/Desktop/Labs_6th sem/OS_Lab"  # e.g., "/home/user/myproject"

def run_git_command(command, cwd=None):
    """Run a Git command in the specified directory"""
    return subprocess.run(command, capture_output=True, text=True, cwd=cwd or DIRECTORY)

def get_untracked_files():
    """Get a list of untracked (new) files"""
    result = run_git_command(["git", "ls-files", "--others", "--exclude-standard"])
    return result.stdout.splitlines()

def get_modified_files():
    """Get a list of modified files"""
    result = run_git_command(["git", "status", "--porcelain"])
    return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" M ")]

def get_deleted_files():
    """Get a list of deleted files"""
    result = run_git_command(["git", "status", "--porcelain"])
    return [line.split()[-1] for line in result.stdout.splitlines() if line.startswith(" D ")]

def get_file_diff(filename):
    """Get the git diff for a modified file"""
    result = run_git_command(["git", "diff", "--cached", "--", filename])
    if not result.stdout:  # If not staged, get the unstaged diff
        result = run_git_command(["git", "diff", "--", filename])
    return result.stdout

def call_together_api(diff):
    """Call Together AI API to generate a commit message based on the diff"""
    prompt = (
        "You are a senior developer writing a concise, realistic Git commit message. "
        "Based on the following git diff, write a detailed and professional commit message "
        "describing the changes. Include specifics about what was added, removed, or modified, "
        "and why. Keep it under 80 characters if possible:\n\n"
        f"```diff\n{diff}\n```"
    )

    response = client.chat.completions.create(
        model="meta-llama/Llama-3.1-70B-Instruct-Turbo",  # Using Llama 3.1 70B as requested
        messages=[{"role": "user", "content": prompt}],
        max_tokens=100,
        temperature=0.7,
        top_p=0.9,
    )
    return response.choices[0].message.content.strip()

def generate_fallback_message(filename, is_deleted=False):
    """Generate a fallback commit message for new or deleted files"""
    file_name = os.path.basename(filename)
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    if is_deleted:
        return f"Removed {file_name} - obsolete as of {timestamp}"
    
    file_ext = os.path.splitext(filename)[1].lower()
    if file_ext in [".py", ".js"]:
        return f"Added {file_name} with initial implementation ({timestamp})"
    elif file_ext in [".md", ".txt"]:
        return f"Created {file_name} with initial draft ({timestamp})"
    else:
        return f"Introduced {file_name} to the project ({timestamp})"

def generate_commit_message(filename, is_deleted=False):
    """Generate a commit message, using LLM for modified files or fallback for others"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    if is_deleted:
        return generate_fallback_message(filename, is_deleted=True)
    
    if filename in get_modified_files():
        diff = get_file_diff(filename)
        if diff:
            try:
                message = call_together_api(diff)
                return f"{message} ({timestamp})"
            except Exception as e:
                print(f"Failed to get LLM message for {filename}: {e}")
                return f"Updated {os.path.basename(filename)} with recent changes ({timestamp})"
    
    # Fallback for untracked (new) files
    return generate_fallback_message(filename)

def commit_and_push():
    # Ensure the directory exists and is a Git repo
    if not os.path.isdir(DIRECTORY) or not os.path.isdir(os.path.join(DIRECTORY, ".git")):
        print(f"Error: {DIRECTORY} is not a valid Git repository.")
        return

    modified_files = get_modified_files()
    untracked_files = get_untracked_files()
    deleted_files = get_deleted_files()
    all_files = modified_files + untracked_files + deleted_files

    if not all_files:
        print("No new, modified, or deleted files to commit.")
        return

    for file in all_files:
        if file in deleted_files:
            run_git_command(["git", "rm", file])  # Remove deleted files
            commit_message = generate_commit_message(file, is_deleted=True)
        else:
            run_git_command(["git", "add", file])  # Add new/modified files
            commit_message = generate_commit_message(file)

        run_git_command(["git", "commit", "-m", commit_message])

        # Push immediately after each commit
        run_git_command(["git", "push", "origin", "main"])  # Change branch if necessary
        time.sleep(2)  # Small delay to mimic manual commits

    print("All changes committed and pushed successfully!")

if __name__ == "__main__":
    commit_and_push()
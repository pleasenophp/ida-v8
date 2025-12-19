# Pre-build script to update version in src/version.h from package.json
# Calculates patch version based on git commits since the corresponding tag

# Read package.json to get the version
try {
    $packageJsonPath = "package.json"
    $packageJson = Get-Content -Path $packageJsonPath -Raw | ConvertFrom-Json
    $packageVersion = $packageJson.version
    
    Write-Host "Found version in package.json: $packageVersion"
    
    # Parse the version to get major.minor.patch components
    if ($packageVersion -match "^(\d+)\.(\d+)\.(\d+)$") {
        $major = $matches[1]
        $minor = $matches[2]
        $basePatch = [int]$matches[3]
        
        Write-Host "Parsed version - Major: $major, Minor: $minor, Base Patch: $basePatch"
        
        # Check if the corresponding git tag exists
        $tagName = "v$packageVersion"
        $tagExists = git tag --list $tagName
        
        if ($tagExists) {
            Write-Host "Found git tag: $tagName"
            
            # Count commits since this tag
            $commitsSinceTag = git rev-list --count "$tagName..HEAD"
            $commitCount = [int]$commitsSinceTag
            
            Write-Host "Commits since tag $tagName`: $commitCount"
            
            # Calculate new patch version
            $newPatch = $basePatch + $commitCount
            $finalVersion = "$major.$minor.$newPatch"
            
            Write-Host "Calculated final version: $finalVersion"
        }
        else {
            Write-Host "Git tag $tagName not found, using package.json version as-is"
            $finalVersion = $packageVersion
        }
    }
    else {
        Write-Warning "Version format not recognized (expected x.y.z), using package.json version as-is"
        $finalVersion = $packageVersion
    }
    
    # Path to version.h file
    $versionHeaderPath = "src\version.h"
    
    $versionContent = @"
#pragma once
#define IDA_VERSION "$finalVersion"
"@
    
    # Write the content to version.h
    $versionContent | Out-File -FilePath $versionHeaderPath -Encoding UTF8
    
    Write-Host "Updated IDA_VERSION to: $finalVersion"
    Write-Host "Version header updated successfully!"
    
}
catch {
    Write-Error "Failed to update version: $($_.Exception.Message)"
    exit 1
}
